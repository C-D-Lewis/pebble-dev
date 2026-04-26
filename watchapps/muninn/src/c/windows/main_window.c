#include "main_window.h"

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
  #define EYE_RECT GRect(96, 11, 3, 3)
  #define BRAID_H 18
  #define ICON_SIZE 28
  #define ROW_2_LABEL "  Last charge   Next charge"
#elif defined(PBL_PLATFORM_CHALK)
  #define EYE_RECT GRect(91, 9, 2, 2)
  #define BRAID_H 14
  #define ICON_SIZE 24
  #define ROW_2_LABEL "      Last charge  Next charge"
#else
  #define EYE_RECT GRect(72, 7, 2, 2)
  #define BRAID_H 14
  #define ICON_SIZE 24
  #define ROW_2_LABEL " Last chrg     Next chrg"
#endif

#define DIV_W 2

static Window *s_window;
static Layer *s_canvas_layer;

static GBitmap *s_mascot_bitmap, *s_batt_bitmap;
static AppTimer *s_blink_timer;
static char s_remaining_buff[16];
static char s_rate_buff[16];
static char s_fmt_lc_buff[8];
static char s_battery_buff[8];
static char s_nc_buff[16];
static char s_reading_buff[8];
static char s_row_1_labels_buff[40];
static int s_blink_budget, s_days_remaining, s_rate, s_anim_days, s_anim_rate;
static bool s_is_blinking, s_is_enabled;

static void update_labels(int days) {
#if defined(PBL_PLATFORM_EMERY)
  const char *template = "  Day%s left        %% per day";
#elif defined(PBL_PLATFORM_CHALK)
  const char *template = "      Day%s left         %% per day";
#else
  const char *template = " Day%s left      %% per day";
#endif
  snprintf(s_row_1_labels_buff, sizeof(s_row_1_labels_buff), template, days == 1 ? " " : "s");
}

//////////////////////////////////////////// Animations ////////////////////////////////////////////

static void update_anim_text() {
#ifdef FEATURE_ANIMATIONS
  const bool animating = util_is_animating();
#else
  const bool animating = false;
#endif
  const int days = animating ? s_anim_days : s_days_remaining;
  const int rate = animating ? s_anim_rate : s_rate;

  // Separate tens and units for decimal point display
  const int days_10x = data_calculate_days_remaining(true);
  int tens = days_10x / 10;
  if (animating || tens >= 10) {
    snprintf(s_remaining_buff, sizeof(s_remaining_buff), "%d", days);
  } else {
    // Show with more precision if not animating and less than 10 days remaining
    snprintf(s_remaining_buff, sizeof(s_remaining_buff), "%d.%d", tens, days_10x % 10);
  }

  const int rate_100x = data_calculate_avg_discharge_rate_x100(false);
  tens = rate_100x / 100;
  if (animating || tens >= 10) {
    snprintf(s_rate_buff, sizeof(s_rate_buff), "%d", rate);
  } else {
    snprintf(s_rate_buff, sizeof(s_rate_buff), "%d.%d", tens, (rate_100x % 100) / 10);
  }
}

#ifdef FEATURE_ANIMATIONS
static void anim_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_days = util_anim_percentage(dist_normalized, s_days_remaining);
  s_anim_rate = util_anim_percentage(dist_normalized, s_rate);

  update_anim_text();
  layer_mark_dirty(s_canvas_layer);
}

static void anim_teardown(Animation *anim) {
  update_anim_text();
  layer_mark_dirty(s_canvas_layer);
}
#endif

///////////////////////////////////////////// Blinking /////////////////////////////////////////////

static void schedule_blink();

static void unblink_handler(void *context) {
  s_is_blinking = false;
  layer_mark_dirty(s_canvas_layer);

  schedule_blink();
}

static void blink_handler(void *context) {
  s_is_blinking = true;
  layer_mark_dirty(s_canvas_layer);

  s_blink_timer = app_timer_register(500, unblink_handler, NULL);
}

static void schedule_blink() {
  // Only blink a few times in case app is left open
  if (s_blink_budget == 0) {
    s_blink_timer = NULL;
    return;
  }

  s_blink_budget--;
  s_blink_timer = app_timer_register(700 + (rand() % 3000), blink_handler, NULL);
}

static void cancel_blink() {
  if (s_blink_timer) {
    app_timer_cancel(s_blink_timer);
    s_blink_timer = NULL;
  }

  s_is_blinking = false;
  layer_mark_dirty(s_canvas_layer);
}

///////////////////////////////////////////// Handlers /////////////////////////////////////////////

static void update_data() {
  PersistData *persist_data = data_get_persist_data();

  time_t wakeup_ts;
  const int wakeup_id = persist_data->wakeup_id;
  const bool wakeup_id_set = util_is_not_status(wakeup_id);
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);
  s_is_enabled = wakeup_id_set && found;
  if (wakeup_id_set && !found) {
    data_set_error("Scheduled wakeup was not found");
  }

  cancel_blink();

  // Battery now
  BatteryChargeState state = battery_state_service_peek();
  snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", state.charge_percent);

  if (!s_is_enabled) {
    // Anything that makes predictions should not be shown if not actively monitoring
    snprintf(s_reading_buff, sizeof(s_reading_buff), "--:--");
    snprintf(s_remaining_buff, sizeof(s_remaining_buff), "--");
    snprintf(s_rate_buff, sizeof(s_rate_buff), "--");
    snprintf(s_nc_buff, sizeof(s_nc_buff), "--");
  } else {
    schedule_blink();

    // Days remaining
    s_days_remaining = data_calculate_days_remaining(false);
    if (util_is_not_status(s_days_remaining)) {
#ifdef FEATURE_ANIMATIONS
      // Handled in animation
      snprintf(s_remaining_buff, sizeof(s_remaining_buff), "--");
#else
      // Set statically
      update_anim_text();
#endif
    } else {
      snprintf(s_remaining_buff, sizeof(s_remaining_buff), "--");
    }

    update_labels(s_days_remaining);

    // Rate per day
    s_rate = data_calculate_avg_discharge_rate_x100(false) / 100;
    if (util_is_not_status(s_rate)) {
#ifdef FEATURE_ANIMATIONS
      snprintf(s_rate_buff, sizeof(s_rate_buff), "--");
#else
      update_anim_text();
#endif
    } else {
      snprintf(s_rate_buff, sizeof(s_rate_buff), "--");
    }

    // "Xd" last charge time
    const int last_charge_ts = persist_data->last_charge_time;
    if (util_is_not_status(last_charge_ts)) {
      util_fmt_time_ago(
        last_charge_ts,
        &s_fmt_lc_buff[0],
        sizeof(s_fmt_lc_buff)
      );
    } else {
      snprintf(s_fmt_lc_buff, sizeof(s_fmt_lc_buff), "--");
    }

    // "26/4" next charge time
    time_t next_charge_ts = data_get_next_charge_time();
    if (util_is_not_status(next_charge_ts)) {
      struct tm *nc_info = localtime(&next_charge_ts);
      if (persist_data->reverse_dates) {
        snprintf(s_nc_buff, sizeof(s_nc_buff), "%d/%d", nc_info->tm_mon + 1, nc_info->tm_mday);
      } else {
        snprintf(s_nc_buff, sizeof(s_nc_buff), "%d/%d", nc_info->tm_mday, nc_info->tm_mon + 1);
      }
    } else {
      snprintf(s_nc_buff, sizeof(s_nc_buff), "--");
    }

    // Next reading
    util_fmt_time(wakeup_ts, &s_reading_buff[0], sizeof(s_reading_buff));
  }

#ifdef FEATURE_ANIMATIONS
  if (data_calculate_avg_discharge_rate_x100(false) != STATUS_EMPTY) {
    // If data to show, begin smooth animation
    static AnimationImplementation anim_implementation = {
      .update = anim_update,
      .teardown = anim_teardown
    };
    util_animate(500, 50, &anim_implementation, true);
  }
#endif
}

static void draw_text(GContext *ctx, char *ptr, int font_id, int x, int y) {
  graphics_draw_text(
    ctx,
    ptr,
    scl_get_font(font_id),
    GRect(x, y, PS_DISP_W, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_antialiased(ctx, false);

  // Divider braid
  const GRect braid_rect = GRect(0, scl_y(160), PS_DISP_W - ACTION_BAR_W, BRAID_H);
  util_draw_braid(ctx, braid_rect);

  // Skyline
  const bool is_night = util_get_is_night();
  util_draw_skyline(ctx, is_night);

  // Hints
  util_draw_button_hints(ctx, (bool[3]){true, true, true});

  // Mascot
  if (s_mascot_bitmap != NULL) {
    bitmaps_destroy_ptr(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
  s_mascot_bitmap = bitmaps_get(util_get_mascot_res_id(s_is_enabled, is_night));
  graphics_draw_bitmap_in_rect(
    ctx,
    s_mascot_bitmap,
    GRect(
      scl_x_pp({.o = 400, .c = 430, .e = 400}),
      scl_y_pp({.o = 20, .c = 25, .e = 25}),
      MASCOT_SIZE,
      MASCOT_SIZE
    )
  );

  // Blink Muninn's eye
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, is_night ? GColorWhite : GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);
  }

  // Bitmaps
  BatteryChargeState state = battery_state_service_peek();
  if (s_batt_bitmap != NULL) {
    bitmaps_destroy_ptr(s_batt_bitmap);
    s_batt_bitmap = NULL;
  }
  s_batt_bitmap = bitmaps_get(util_get_battery_resource_id(state.charge_percent));

  graphics_context_set_text_color(ctx, GColorBlack);

  // Not enabled hint
  if (!s_is_enabled) {
    graphics_draw_text(
      ctx,
      "Hold the Up button to wake Muninn!",
      scl_get_font(SFI_MediumBold),
      GRect(4, scl_y_pp({.o = 360, .c = 370, .e = 400}), PS_DISP_W - ACTION_BAR_W - 8, 100),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL
    );
    return;
  }

  // Left column
  int icon_x = scl_x_pp({.o = 40, .c = 100, .e = 50});
  int icon_y = scl_y_pp({.o = 275, .e = 285});
  int text_x = icon_x + ICON_SIZE + scl_x(10);
  int text_y = icon_y - scl_y_pp({.o = 40, .e = 15});

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_REMAINING),
    GRect(icon_x, icon_y, ICON_SIZE, ICON_SIZE)
  );
  draw_text(ctx, s_remaining_buff, SFI_LargeBold, text_x + scl_x(10), text_y);

  icon_y = scl_y_pp({.o = 560, .e = 560});
  text_y = icon_y - scl_y_pp({.o = 25, .e = 20});

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_LAST_CHARGE),
    GRect(icon_x, icon_y, ICON_SIZE, ICON_SIZE)
  );
  draw_text(ctx, s_fmt_lc_buff, SFI_Medium, text_x, text_y);

#if !defined(PBL_PLATFORM_CHALK)
  icon_y = scl_y_pp({.o = 850, .e = 870});
  text_y = icon_y - scl_y_pp({.o = 30, .e = 20});

  graphics_draw_bitmap_in_rect(
    ctx,
    s_batt_bitmap,
    GRect(icon_x - scl_x(15), icon_y, ICON_SIZE, ICON_SIZE)
  );
  draw_text(ctx, s_battery_buff, SFI_Medium, text_x - scl_x(20), text_y);
#endif

  // Right column
  icon_x = scl_x_pp({.o = 480, .c = 500});
  icon_y = scl_y_pp({.o = 275, .e = 285});
  text_x = icon_x + ICON_SIZE + scl_x(10);
  text_y = icon_y - scl_y_pp({.o = 40, .e = 15});

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_RATE),
    GRect(icon_x, icon_y, ICON_SIZE, ICON_SIZE)
  );
  draw_text(ctx, s_rate_buff, SFI_LargeBold, text_x + scl_x(10), text_y);

  icon_x = scl_x_pp({.o = 460, .c = 500});
  icon_y = scl_y_pp({.o = 560, .e = 560});
  text_x = icon_x + ICON_SIZE - scl_x(10);
  text_y = icon_y - scl_y_pp({.o = 25, .e = 20});
  
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_NEXT_CHARGE),
    GRect(icon_x, icon_y, ICON_SIZE, ICON_SIZE)
  );
  draw_text(ctx, s_nc_buff, SFI_Medium, text_x, text_y);

  icon_x = scl_x_pp({.o = 460, .c = 320});
  icon_y = scl_y_pp({.o = 850, .c = 830, .e = 870});
  text_x = icon_x + ICON_SIZE;
  text_y = icon_y - scl_y_pp({.o = 30, .e = 20});

#if defined(PBL_PLATFORM_APLITE)
  // Draw clock instead of bitmap for 'next sample'
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  const int center_x = icon_x + (ICON_SIZE / 2);
  const int center_y = icon_y + (ICON_SIZE / 2);
  graphics_draw_circle(ctx, GPoint(center_x, center_y), (ICON_SIZE / 2) - 4);

  // Draw hands
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(center_x, center_y), GPoint(center_x, center_y - 5));
  graphics_draw_line(ctx, GPoint(center_x, center_y), GPoint(center_x + 5, center_y));
#else
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_READING),
    GRect(icon_x, icon_y, ICON_SIZE, ICON_SIZE)
   );
#endif
  draw_text(ctx, s_reading_buff, SFI_Medium, text_x, text_y);

  // Bottom divider - DIV_W - just above bottom two icons
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    GRect(0, scl_y_pp({.o = 835, .c = 810, .e = 840}), PS_DISP_W - ACTION_BAR_W, DIV_W),
    0,
    GCornerNone
  );

  // Row labels
  draw_text(ctx, s_row_1_labels_buff, SFI_Small, 0, scl_y_pp({.o = 390, .e = 390}));
  draw_text(ctx, ROW_2_LABEL, SFI_Small, 0, scl_y_pp({.o = 680, .c = 665, .e = 660}));
}

////////////////////////////////////////////// Clicks //////////////////////////////////////////////

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  PersistData *persist_data = data_get_persist_data();

  const bool should_enable = !util_is_not_status(persist_data->wakeup_id);
  if (should_enable) {
    s_blink_budget = 5;
    schedule_blink();

    wakeup_schedule_next();
  } else {
    wakeup_unschedule();
  }

  vibes_long_pulse();
  update_data();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef FEATURE_ANIMATIONS
  if (!util_is_animating())
#endif
    menu_window_push();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
#ifdef FEATURE_ANIMATIONS
  if (!util_is_animating())
#endif
    graph_window_push();
}

static void click_config_provider(void *context) {
  window_long_click_subscribe(BUTTON_ID_UP, 1000, up_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

////////////////////////////////////////////// Window //////////////////////////////////////////////

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  update_data();

  // APP_LOG(APP_LOG_LEVEL_INFO, "Heap %d", heap_bytes_free());
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void window_appear(Window *window) {
  // Some settings need redraw like date format
  if (!s_canvas_layer) return;
  
  update_data();
  layer_mark_dirty(s_canvas_layer);
}

static void window_disappear(Window *window) {
  bitmaps_destroy_all();

  // Note images that will loaded in canvas update proc
  s_mascot_bitmap = NULL;
  s_batt_bitmap = NULL;

  // APP_LOG(APP_LOG_LEVEL_INFO, "wd %d B", heap_bytes_free());
  // bitmap_log_allocated_count();
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear,
      .disappear = window_disappear,
    });
    window_set_click_config_provider(s_window, click_config_provider);

    s_blink_budget = 5;
  }

  window_stack_push(s_window, true);

  data_assume_last_charge_time();
}

void main_window_update() {
  update_data();
}
