#include "main_window.h"

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
  #define MASCOT_SIZE 30
  #define EYE_RECT GRect(96, 11, 3, 3)
  #define BRAID_H 18
  #define ICON_SIZE 28
  #define CLOUD_SIZE GSize(30, 19)
#elif defined(PBL_PLATFORM_CHALK)
  #define MASCOT_SIZE 24
  #define EYE_RECT GRect(91, 9, 2, 2)
  #define BRAID_H 14
  #define ICON_SIZE 24
  #define CLOUD_SIZE GSize(24, 15)
#else
  #define MASCOT_SIZE 24
  #define EYE_RECT GRect(72, 7, 2, 2)
  #define BRAID_H 14
  #define ICON_SIZE 24
  #define CLOUD_SIZE GSize(24, 15)
#endif

#define BRAID_Y scl_y_pp({.o = 280, .e = 275})

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
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
#elif defined(PBL_PLATFORM_CHALK)
  const char *template = "     Day%s left        Est. %%/day";
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

  cancel_blink();

  // Battery now
  BatteryChargeState state = battery_state_service_peek();
  static char s_battery_buff[8];
  snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", state.charge_percent);
  // text_layer_set_text(s_battery_layer, "100%");
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
    // text_layer_set_text(s_reading_layer, "00:00");
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

static uint32_t get_mascot_res_id(bool is_nighttime) {
  if (is_nighttime) {
    return s_is_enabled ? RESOURCE_ID_AWAKE_HEAD_INV : RESOURCE_ID_ASLEEP_HEAD_INV;
  }

  return s_is_enabled ? RESOURCE_ID_AWAKE_HEAD : RESOURCE_ID_ASLEEP_HEAD;
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_antialiased(ctx, false);

  // Divider braid
  const GRect braid_rect = GRect(0, BRAID_Y, PS_DISP_W - ACTION_BAR_W, BRAID_H);
  util_draw_braid(ctx, braid_rect);

  // Row dividers
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, LINE_W);

  // Vertical
  const int v_div_x = (PS_DISP_W / 2) - scl_x(40);
  const int v_div_y = scl_y_pp({.o = 365, .c = 350, .e = 355});
  const int v_div_h = scl_y_pp({.o = 700, .c = 475, .e = 700});
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
  const int row_3_div_y = scl_y_pp({.o = 830, .c = 835, .e = 835});
  graphics_draw_line(
    ctx,
    GPoint(0, row_3_div_y),
    GPoint(PS_DISP_W - ACTION_BAR_W, row_3_div_y)
  );

  // Status BG
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, scl_grect(0, 160, 940, 120), 0, GCornerNone);

  util_draw_button_hints(ctx, (bool[3]){true, true, true});

  const time_t now = time(NULL);
  const struct tm *now_info = localtime(&now);
  const bool is_nighttime = now_info->tm_hour < 6 || now_info->tm_hour >= 18;

  // Mascot banner
  graphics_context_set_fill_color(ctx, is_nighttime ? GColorBlack : GColorWhite);
  const uint8_t skyline_y = scl_y(160);
  const GRect skyline_rect = GRect(0, 0, PS_DISP_W - ACTION_BAR_W, skyline_y);
  graphics_fill_rect(ctx, skyline_rect, 0, GCornerNone);

  // Mascot
  if (s_mascot_bitmap != NULL) {
    bitmaps_destroy_ptr(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
  uint32_t mascot_res_id = get_mascot_res_id(is_nighttime);
  s_mascot_bitmap = bitmaps_get(
    mascot_res_id
  );
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
    graphics_context_set_fill_color(ctx, is_nighttime ? GColorWhite : GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);
  }

  // Decorate mascot banner
  if (is_nighttime) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    // Draw a couple of stars in black
    const GPoint stars[] = {
      GPoint(scl_x(40), scl_y(50)),
      GPoint(scl_x(90), scl_y(110)),
      GPoint(scl_x(300), scl_y(80)),
      GPoint(scl_x(200), scl_y(50)),
      GPoint(scl_x(800), scl_y(70)),
      GPoint(scl_x(720), scl_y(90)),
      GPoint(scl_x(610), scl_y(30)),
      GPoint(scl_x(880), scl_y(110))
    };
    const uint8_t num_stars = 8;
    const uint8_t star_max_size = scl_x(15);
    for (int i = 0; i < num_stars; i++) {
      const uint8_t size = (i % star_max_size) + 1;
      graphics_fill_rect(ctx, GRect(stars[i].x, stars[i].y, size, size), 0, GCornerNone);
    }

    // Skyline below mascot
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(
      ctx,
      GPoint(0, skyline_y),
      GPoint(PS_DISP_W - ACTION_BAR_W - 1, skyline_y)
    );
  } else {
    // Clouds and Huginn
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_CLOUD),
      GRect(scl_x_pp({.o = 60, .c = 240, .e = 80}), scl_y(20), CLOUD_SIZE.w, CLOUD_SIZE.h)
    );
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_CLOUD),
      GRect(scl_x(680), scl_y(50), CLOUD_SIZE.w, CLOUD_SIZE.h)
    );
#if !defined(PBL_PLATFORM_CHALK)
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_BIRD),
      GRect(scl_x(270), scl_y(30), 16, 16)
    );
#endif
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
    GRect(scl_x_pp({.o = 40, .c = 80}), scl_y_pp({.o = 390, .e = 385}), ICON_SIZE, ICON_SIZE)
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
    GRect(scl_x_pp({.o = 30, .c = 140}), scl_y_pp({.o = 665, .e = 670}), ICON_SIZE, ICON_SIZE)
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_NEXT_CHARGE),
    GRect(
      scl_x_pp({.o = 480, .c = 500, .e = 490}),
      scl_y_pp({.o = 665, .e = 670}),
      ICON_SIZE,
      ICON_SIZE
    )
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

  const int icon_x = scl_x(470);
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
    GRect(
      scl_x_pp({.o = 470, .c = 330, .e = 490}),
      scl_y_pp({.o = 850, .c = 840, .e = 860}),
      ICON_SIZE,
      ICON_SIZE
    )
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

  s_desc_layer = util_make_text_layer(
    GRect(0, scl_y_pp({.o = 145, .e = 135}), scl_x_pp({.o = 940, .c = 1000}), scl_y(150)),
    scl_get_font(SFI_Small)
  );
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_desc_layer, GColorWhite);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Row 1
  int row_x = scl_x_pp({.o = 40, .c = 80, .e = 50});
  int row_y = scl_y_pp({.o = 350, .e = 365});
#if defined(PBL_PLATFORM_APLITE)
  row_x += scl_x(100);
#else
  row_x += scl_x_pp({.o = 190, .e = 150});
#endif

  s_remaining_layer = util_make_text_layer(
    GRect(row_x, row_y, PS_DISP_W, 100),
    scl_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += scl_x_pp({.o = 485, .c = 445, .e = 485});

  s_rate_layer = util_make_text_layer(
    GRect(row_x, row_y, PS_DISP_W, 100),
    scl_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  s_row_1_subtitle_layer = util_make_text_layer(
    GRect(2, row_y + scl_y_pp({.o = 155, .e = 120}), PS_DISP_W - ACTION_BAR_W, 40),
    scl_get_font(SFI_Small)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_row_1_subtitle_layer));

  // Row 2
  row_x = scl_x_pp({.o = 150, .c = 230});
  row_y = scl_y_pp({.o = 635, .c = 650, .e = 650});

  const int text_ico_nudge = scl_x_pp({.o = 60, .e = 40});
  s_last_charge_layer = util_make_text_layer(
    GRect(row_x + text_ico_nudge, row_y, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_last_charge_layer));

  row_x += scl_x_pp({.o = 450, .c = 380, .e = 450});

  s_next_charge_layer = util_make_text_layer(
    GRect(row_x, row_y, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_next_charge_layer));

  // Row 3
  row_x = scl_x(150);
  row_y = scl_y_pp({.o = 815, .c = 810, .e = 840});

  s_battery_layer = util_make_text_layer(
    GRect(row_x, row_y, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
#if !defined(PBL_PLATFORM_CHALK)
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));
#endif

  row_x += scl_x_pp({.o = 470, .c = 280, .e = 480});

  const int x_nudge = scl_x_pp({.o = 20, .e = 10});
  s_reading_layer = util_make_text_layer(
    GRect(row_x + x_nudge, row_y, PS_DISP_W, 100),
    scl_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Hint for when Muninn is asleep (topmost)
  const GRect hint_rect = GRect(
    0,
    scl_y_pp({.o = 360, .e = 355}),
    PS_DISP_W - ACTION_BAR_W,
    scl_y_pp({.o = 470, .e = 475})
  );
  s_hint_layer = util_make_text_layer(hint_rect, scl_get_font(SFI_Medium));
  text_layer_set_background_color(s_hint_layer, GColorWhite);
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hint_layer, "Hold the Up button\nto wake Muninn.");
  layer_add_child(root_layer, text_layer_get_layer(s_hint_layer));

  update_data();

  APP_LOG(APP_LOG_LEVEL_INFO, "Heap %d", heap_bytes_free());
}

static void window_unload(Window *window) {
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

  APP_LOG(APP_LOG_LEVEL_INFO, "wd %d B", heap_bytes_free());
  bitmap_log_allocated_count();
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
