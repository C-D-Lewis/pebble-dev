#include "main_window.h"

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
  #define MASCOT_SIZE 30
  #define EYE_RECT GRect(28, 11, 4, 4)
  #define BRAID_H 18
  #define ICON_SIZE 28
#else
  #define MASCOT_SIZE 24
  #define EYE_RECT GRect(20, 7, 4, 4)
  #define BRAID_H 14
  #define ICON_SIZE 24
#endif

#define BRAID_Y scl_y_pp({.o = 280, .e = 275})

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
  *s_status_layer,
  *s_desc_layer,
  *s_battery_layer,
  *s_reading_layer,
  *s_row_1_subtitle_layer,
  *s_remaining_layer,
  *s_rate_layer,
  *s_hint_layer,
  *s_last_charge_layer,
  *s_next_charge_layer;

static GBitmap *s_mascot_bitmap, *s_batt_bitmap;
static AppTimer *s_blink_timer;
static int s_blink_budget;
static bool s_is_blinking, s_is_enabled;
static int s_days_remaining, s_rate, s_anim_days, s_anim_rate;

static void update_subtitle(int days) {
  static char s_subtitle_buff[40];
#if defined(PBL_PLATFORM_EMERY)
  const char *template = "   Day%s left        Est. %%/day";
#else
  const char *template = "     Day%s          Est. %%/d";
#endif
  snprintf(s_subtitle_buff, sizeof(s_subtitle_buff), template, days == 1 ? " " : "s");
  text_layer_set_text(s_row_1_subtitle_layer, s_subtitle_buff);
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

  // Update text layers
  static char s_remaining_buff[8];
  snprintf(s_remaining_buff, sizeof(s_remaining_buff), days < 10 ? "0%d" : "%d", days);
  text_layer_set_text(s_remaining_layer, s_remaining_buff);

  static char s_rate_buff[8];
  snprintf(s_rate_buff, sizeof(s_rate_buff), rate < 10 ? "0%d" : "%d", rate);
  text_layer_set_text(s_rate_layer, s_rate_buff);
}

#ifdef FEATURE_ANIMATIONS
static void anim_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_days = util_anim_percentage(dist_normalized, s_days_remaining);
  s_anim_rate = util_anim_percentage(dist_normalized, s_rate);

  update_anim_text();
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

  text_layer_set_text(s_status_layer, s_is_enabled ? "AWAKE" : "ASLEEP");

  cancel_blink();

  // Battery now
  BatteryChargeState state = battery_state_service_peek();
  static char s_battery_buff[8];
  snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buff);

  // Current status
  text_layer_set_text(s_desc_layer, util_get_status_string());

  if (!s_is_enabled) {
    // Anything that makes predictions should not be shown if not actively monitoring
    text_layer_set_text(s_reading_layer, " --:--");
    text_layer_set_text(s_remaining_layer, "--");
    text_layer_set_text(s_rate_layer, "--");
    text_layer_set_text(s_next_charge_layer, "--");

    layer_set_hidden(text_layer_get_layer(s_hint_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_hint_layer), true);
    schedule_blink();

    // Days remaining
    s_days_remaining = data_calculate_days_remaining();
    if (util_is_not_status(s_days_remaining)) {
#ifdef FEATURE_ANIMATIONS
      // Handled in animation
      text_layer_set_text(s_remaining_layer, "--");
#else
      // Set statically
      update_anim_text();
#endif
    } else {
      text_layer_set_text(s_remaining_layer, "--");
    }

    update_subtitle(s_days_remaining);

    // Rate per day
    s_rate = data_calculate_avg_discharge_rate(false);
    if (util_is_not_status(s_rate)) {
#ifdef FEATURE_ANIMATIONS
      text_layer_set_text(s_rate_layer, "--");
#else
      update_anim_text();
#endif
    } else {
      text_layer_set_text(s_rate_layer, "--");
    }

    // "Xd" last charge time
    static char s_fmt_lc_buff[8];
    const int last_charge_ts = persist_data->last_charge_time;
    if (util_is_not_status(last_charge_ts)) {
      util_fmt_time_ago(
        last_charge_ts,
        &s_fmt_lc_buff[0],
        sizeof(s_fmt_lc_buff)
      );
      text_layer_set_text(s_last_charge_layer, s_fmt_lc_buff);
    } else {
      text_layer_set_text(s_last_charge_layer, "--");
    }

    // "12 Dec" next charge time
    static char s_nc_buff[8];
    time_t next_charge_ts = data_get_next_charge_time();
    if (util_is_not_status(next_charge_ts)) {
      struct tm *nc_info = localtime(&next_charge_ts);
      strftime(s_nc_buff, sizeof(s_nc_buff), "%d %b", nc_info);
      text_layer_set_text(s_next_charge_layer, s_nc_buff);
    } else {
      text_layer_set_text(s_next_charge_layer, "--");
    }

    // Next reading
    static char s_wakeup_buff[8];
    util_fmt_time(wakeup_ts, &s_wakeup_buff[0], sizeof(s_wakeup_buff));
    // text_layer_set_text(s_reading_layer, "22:00");
    text_layer_set_text(s_reading_layer, s_wakeup_buff);
  }

#ifdef FEATURE_ANIMATIONS
  if (data_calculate_avg_discharge_rate(false) != STATUS_EMPTY) {
    // If data to show, begin smooth animation
    static AnimationImplementation anim_implementation = { .update = anim_update };
    util_animate(500, 50, &anim_implementation, true);
  }
#endif
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Divider braid
  const GRect braid_rect = GRect(0, BRAID_Y, PS_DISP_W - ACTION_BAR_W, BRAID_H);
  util_draw_braid(ctx, braid_rect);

  // Row dividers
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, LINE_W);

  // Vertical
  const int v_div_x = (PS_DISP_W / 2) - scl_x(40);
  const int v_div_y = scl_y_pp({.o = 365, .e = 355});
  const int v_div_h = scl_y_pp({.o = 465, .e = 470});
  graphics_draw_line(
    ctx,
    GPoint(v_div_x, v_div_y),
    GPoint(v_div_x, v_div_y + v_div_h)
  );

  // Horizontal below row 1
  const int row_2_div_y = scl_y_pp({.o = 650, .e = 630});
  graphics_draw_line(
    ctx,
    GPoint(0, row_2_div_y),
    GPoint(PS_DISP_W - (ACTION_BAR_W), row_2_div_y)
  );

  // Horizontal below row 2
  const int row_3_div_y = scl_y_pp({.o = 830, .e = 835});
  graphics_draw_line(
    ctx,
    GPoint(0, row_3_div_y),
    GPoint(PS_DISP_W - ACTION_BAR_W, row_3_div_y)
  );

  // Status BG
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, scl_grect(0, 160, 930, 120), 0, GCornerNone);

  util_draw_button_hints(ctx, (bool[3]){true, true, true});

  // Mascot
  if (s_mascot_bitmap != NULL) {
    bitmaps_destroy_ptr(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
  s_mascot_bitmap = bitmaps_get(
    s_is_enabled ? RESOURCE_ID_AWAKE_HEAD : RESOURCE_ID_ASLEEP_HEAD
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    s_mascot_bitmap,
    GRect(scl_x_pp({.o = 40, .e = 60}), scl_y_pp({.o = 20, .e = 25}), MASCOT_SIZE, MASCOT_SIZE)
  );

  // Blink Muninn's eye
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);
  }

  BatteryChargeState state = battery_state_service_peek();
  if (s_batt_bitmap != NULL) {
    bitmaps_destroy_ptr(s_batt_bitmap);
    s_batt_bitmap = NULL;
  }
  s_batt_bitmap = bitmaps_get(util_get_battery_resource_id(state.charge_percent));

  // Save memory for Aplite, draw icons that don't change without a BitmapLayer
#if !defined(PBL_PLATFORM_APLITE)
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_REMAINING),
    GRect(scl_x(40), scl_y_pp({.o = 390, .e = 385}), ICON_SIZE, ICON_SIZE)
   );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_RATE),
    GRect(scl_x(530), scl_y_pp({.o = 390, .e = 385}), ICON_SIZE, ICON_SIZE)
  );
#endif

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_LAST_CHARGE),
    GRect(scl_x(30), scl_y_pp({.o = 665, .e = 670}), ICON_SIZE, ICON_SIZE)
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_NEXT_CHARGE),
    GRect(scl_x_pp({.o = 480, .e = 490}), scl_y_pp({.o = 665, .e = 670}), ICON_SIZE, ICON_SIZE)
  );

  graphics_draw_bitmap_in_rect(
    ctx,
    s_batt_bitmap,
    GRect(scl_x(10), scl_y_pp({.o = 845, .e = 860}), ICON_SIZE, ICON_SIZE)
  );

#if defined(PBL_PLATFORM_APLITE)
  // Draw clock instead of bitmap for 'next sample'
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  const int icon_x = scl_x(430) + 3;
  const int icon_y = scl_y(845);
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
    GRect(scl_x_pp({.o = 440, .e = 470}), scl_y_pp({.o = 850, .e = 860}), ICON_SIZE, ICON_SIZE)
   );
#endif
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
    settings_window_push();
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

  s_status_layer = util_make_text_layer(
    GRect(0, scl_y_pp({.o = -35, .e = 5}), PS_DISP_W, 100),
    scl_get_font(SFI_LargeBold)
  );
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_status_layer));

  s_desc_layer = util_make_text_layer(
    GRect(0, scl_y_pp({.o = 145, .e = 135}), scl_x(930), scl_y(150)),
    scl_get_font(SFI_Small)
  );
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_desc_layer, GColorWhite);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Row 1
  int row_x = scl_x_pp({.o = 40, .e = 50});
  int row_y = scl_y_pp({.o = 390, .e = 385});
#if defined(PBL_PLATFORM_APLITE)
  int text_ico_off = scl_x_pp({.o = 100, .e = 100});
#else
  int text_ico_off = scl_x_pp({.o = 190, .e = 160});
#endif
  int text_y_off = scl_y_pp({.o = -40, .e = -20});

  s_remaining_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scl_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += scl_x_pp({.o = 485, .e = 485});

  s_rate_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scl_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  s_row_1_subtitle_layer = util_make_text_layer(
    GRect(2, row_y + scl_y_pp({.o = 115, .e = 100}), PS_DISP_W - ACTION_BAR_W, 40),
    scl_get_font(SFI_Small)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_row_1_subtitle_layer));

  // Row 2
  row_x = scl_x(30);
  row_y = scl_y_pp({.o = 665, .e = 670});
  text_ico_off = scl_x_pp({.o = 120, .e = 120});
  text_y_off = scl_y_pp({.o = -35, .e = -25});

  const int text_ico_nudge = scl_x_pp({.o = 60, .e = 40});
  s_last_charge_layer = util_make_text_layer(
    GRect(row_x + text_ico_off + text_ico_nudge, row_y + text_y_off, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_last_charge_layer));

  row_x += scl_x_pp({.o = 450, .e = 450});

  s_next_charge_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_next_charge_layer));

  // Row 3
  row_x = scl_x(10);
  row_y = scl_y_pp({.o = 845, .e = 860});
  text_ico_off = scl_x_pp({.o = 160, .e = 150});
  text_y_off = scl_y_pp({.o = -30, .e = -20});

  s_battery_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  row_x += scl_x_pp({.o = 430, .e = 450});

  const int x_nudge = scl_x_pp({.o = 20, .e = 10});
  s_reading_layer = util_make_text_layer(
    GRect(row_x + text_ico_off + x_nudge, row_y + text_y_off, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Hint for when Muninn is asleep (topmost)
  const GRect hint_rect = GRect(
    0,
    scl_y_pp({.o = 360, .e = 355}),
    scl_x(930),
    scl_y_pp({.o = 470, .e = 475})
  );
  s_hint_layer = util_make_text_layer(hint_rect, scl_get_font(SFI_Medium));
  text_layer_set_background_color(s_hint_layer, GColorWhite);
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hint_layer, "Hold the Up button\nto wake Muninn.");
  layer_add_child(root_layer, text_layer_get_layer(s_hint_layer));

  update_data();

  // APP_LOG(APP_LOG_LEVEL_INFO, "Heap %d", heap_bytes_free());
}

static void window_unload(Window *window) {
  text_layer_destroy(s_status_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_row_1_subtitle_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_reading_layer);
  text_layer_destroy(s_remaining_layer);
  text_layer_destroy(s_rate_layer);
  text_layer_destroy(s_hint_layer);
  text_layer_destroy(s_last_charge_layer);
  text_layer_destroy(s_next_charge_layer);

  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
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
      .disappear = window_disappear,
    });
    window_set_click_config_provider(s_window, click_config_provider);

    s_blink_budget = 5;
  }

  window_stack_push(s_window, true);
}
