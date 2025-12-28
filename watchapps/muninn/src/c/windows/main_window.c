#include "main_window.h"

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
  #define MASCOT_SIZE 30
  #define EYE_RECT GRect(28, 11, 4, 4)
  #define DIV_W 2
  #define BRAID_H 18
  #define ICON_SIZE 28
#else
  #define MASCOT_SIZE 24
  #define EYE_RECT GRect(20, 7, 4, 4)
  #define DIV_W 1
  #define BRAID_H 14
  #define ICON_SIZE 24
#endif

#define ACTION_BAR_W scalable_x(70)
#define BRAID_Y scalable_y_pp(280, 275)
#define HINT_W scalable_x(70)
#define HINT_H scalable_y(200)

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
  *s_status_value_layer,
  *s_desc_layer,
  *s_battery_layer,
  *s_reading_layer,
  *s_row_1_subtitle_layer,
  *s_remaining_layer,
  *s_rate_layer,
  *s_hint_layer,
  *s_last_charge_layer,
  *s_next_charge_layer;
static BitmapLayer
  *s_battery_bmp_layer,
  *s_reading_bmp_layer,
  *s_remaining_bmp_layer,
  *s_rate_bmp_layer,
  *s_last_charge_bmp_layer,
  *s_next_charge_bmp_layer;
static GBitmap
  *s_battery_bitmap,
  *s_reading_bitmap,
  *s_remaining_bitmap,
  *s_rate_bitmap,
  *s_last_charge_bitmap,
  *s_next_charge_bitmap;
#if !defined(PBL_PLATFORM_APLITE)
static GBitmap *s_mascot_bitmap;
static BitmapLayer *s_mascot_layer;
#endif

static AppTimer *s_blink_timer;
static bool s_is_blinking, s_animating;
static int s_blink_budget;
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

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (max * dist_normalized) / ANIMATION_NORMALIZED_MAX;
}

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  if (anim) {
    animation_set_duration(anim, duration);
    animation_set_delay(anim, delay);
    animation_set_curve(anim, AnimationCurveEaseInOut);
    animation_set_implementation(anim, implementation);
    if (handlers) {
      animation_set_handlers(anim, (AnimationHandlers) {
        .started = animation_started,
        .stopped = animation_stopped
      }, NULL);
    }
    animation_schedule(anim);
  }
}

static void anim_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_days = anim_percentage(dist_normalized, s_days_remaining);
  s_anim_rate = anim_percentage(dist_normalized, s_rate);

  // Update text layers
  static char s_remaining_buff[8];
  snprintf(
    s_remaining_buff,
    sizeof(s_remaining_buff),
    s_anim_days < 10 ? "0%d" : "%d",
    s_anim_days
  );
  text_layer_set_text(s_remaining_layer, s_remaining_buff);

  static char s_rate_buff[8];
  snprintf(s_rate_buff, sizeof(s_rate_buff), s_anim_rate < 10 ? "0%d" : "%d", s_anim_rate);
  text_layer_set_text(s_rate_layer, s_rate_buff);
}

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
#if !defined(PBL_PLATFORM_APLITE)
  // Only blink a few times in case app is left open
  if (s_blink_budget == 0) {
    s_blink_timer = NULL;
    return;
  }

  s_blink_budget--;
  s_blink_timer = app_timer_register(700 + (rand() % 3000), blink_handler, NULL);
#endif
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
#if !defined(PBL_PLATFORM_APLITE)
  if (s_mascot_bitmap) {
    bitmaps_destroy(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
#endif
  if (s_battery_bitmap) {
    bitmaps_destroy(s_battery_bitmap);
    s_battery_bitmap = NULL;
  }

  time_t wakeup_ts;
  const int wakeup_id = data_get_wakeup_id();
  const bool is_enabled = util_is_not_status(wakeup_id);
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);
  if (is_enabled && !found) {
    data_set_error("Scheduled wakeup was not found");
  }

#if !defined(PBL_PLATFORM_APLITE)
  s_mascot_bitmap = bitmaps_get(
    is_enabled ? RESOURCE_ID_AWAKE_HEAD : RESOURCE_ID_ASLEEP_HEAD
  );
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
#endif
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");

  cancel_blink();

  // Battery now
  BatteryChargeState state = battery_state_service_peek();
  const int charge_percent = state.charge_percent;
  s_battery_bitmap = bitmaps_get(util_get_battery_resource_id(charge_percent));
  bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
  static char s_battery_buff[8];
  snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buff);

  // Current status
  text_layer_set_text(s_desc_layer, util_get_status_string());

  if (!is_enabled) {
    // Anything that makes predictions should not be shown if not actively monitoring
    text_layer_set_text(s_reading_layer, "--:--");
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
      // Handled in animation
      text_layer_set_text(s_remaining_layer, "--");
    } else {
      text_layer_set_text(s_remaining_layer, "--");
    }

    update_subtitle(s_days_remaining);

    // Rate per day
    s_rate = data_calculate_avg_discharge_rate();
    if (util_is_not_status(s_rate)) {
      // Handled in animation
      text_layer_set_text(s_rate_layer, "--");
    } else {
      text_layer_set_text(s_rate_layer, "--");
    }

    // "Xd" last charge time
    static char s_fmt_lc_buff[8];
    const int last_charge_ts = data_get_last_charge_time();
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
    text_layer_set_text(s_reading_layer, s_wakeup_buff);
  }

  if (data_calculate_avg_discharge_rate() != STATUS_EMPTY) {
    // If data to show, begin smooth animation
    static AnimationImplementation anim_implementation = { .update = anim_update };
    animate(1000, 100, &anim_implementation, true);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Divider braid
  const GRect braid_rect = GRect(0, BRAID_Y, DISPLAY_W - ACTION_BAR_W, BRAID_H);
  util_draw_braid(ctx, braid_rect);

  // Status BG
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    scalable_grect_pp(
      GRect(0, 160, 930, 120),
      GRect(0, 160, 930, 120)
    ),
    0,
    GCornerNone
  );

  // Row dividers
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, DIV_W);
  // Vertical
  const int v_div_x = (DISPLAY_W / 2) - scalable_x(40);
  const int v_div_y = scalable_y_pp(365, 355);
  const int v_div_h = scalable_y_pp(465, 470);
  graphics_draw_line(
    ctx,
    GPoint(v_div_x, v_div_y),
    GPoint(v_div_x, v_div_y + v_div_h)
  );
  // Horizontal below row 1
  const int row_2_div_y = scalable_y_pp(650, 630);
  graphics_draw_line(
    ctx,
    GPoint(0, row_2_div_y),
    GPoint(DISPLAY_W - (ACTION_BAR_W), row_2_div_y)
  );
  // Horizontal below row 2
  const int row_3_div_y = scalable_y_pp(830, 835);
  graphics_draw_line(
    ctx,
    GPoint(0, row_3_div_y),
    GPoint(DISPLAY_W - ACTION_BAR_W, row_3_div_y)
  );

  // Actions BG
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorLightGray));
  GRect actions_rect = GRect(DISPLAY_W - ACTION_BAR_W, 0, ACTION_BAR_W, DISPLAY_H);
  graphics_fill_rect(ctx, actions_rect, 0, GCornerNone);

  const int hint_x = DISPLAY_W - (HINT_W / 2);

  // Enable hint
  const int enable_y = DISPLAY_H / 6 - (HINT_H / 2);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx, GRect(hint_x, enable_y, HINT_W, HINT_H),
    3,
    GCornersAll
  );
  graphics_context_set_fill_color(ctx, GColorWhite);
  const GPoint select_center = {
    .x = hint_x + (HINT_W / 2),
    .y = enable_y + (HINT_H / 2)
  };
  graphics_fill_circle(ctx, select_center, scalable_x(20));

  // Menu hint
  const int menu_y = (DISPLAY_H / 2) - (HINT_H / 2);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx, GRect(hint_x, menu_y, HINT_W, HINT_H),
    3,
    GCornersAll
  );

  // Log hint
  const int log_y = ((5 * DISPLAY_H) / 6) - (HINT_H / 2);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx, GRect(hint_x, log_y, HINT_W, HINT_H),
    3,
    GCornersAll
  );

  // Blink Muninn's eye
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);
  }
}

////////////////////////////////////////////// Clicks //////////////////////////////////////////////

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  const bool should_enable = !util_is_not_status(data_get_wakeup_id());
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
  menu_window_push();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  log_window_push();
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  graph_window_push();
}

static void click_config_provider(void *context) {
  window_long_click_subscribe(BUTTON_ID_UP, 1000, up_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);

  // Debugging info
  window_long_click_subscribe(BUTTON_ID_DOWN, 3000, down_long_click_handler, NULL);
}

////////////////////////////////////////////// Window //////////////////////////////////////////////

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

#if !defined(PBL_PLATFORM_APLITE)
  // Mascot - bitmap set in update_data()
  s_mascot_layer = bitmap_layer_create(
    GRect(scalable_x_pp(40, 60), scalable_y_pp(20, 25), MASCOT_SIZE, MASCOT_SIZE)
  );
  bitmap_layer_set_compositing_mode(s_mascot_layer, GCompOpSet);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_mascot_layer));
#endif

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_status_value_layer = util_make_text_layer(
    scalable_grect_pp(
      GRect(0, -35, 1000, 200),
      GRect(0, 5, 1000, 200)
    ),
    scalable_get_font(SFI_LargeBold)
  );
  text_layer_set_text_alignment(s_status_value_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  s_desc_layer = util_make_text_layer(
    scalable_grect_pp(
      GRect(0, 145, 930, 150),
      GRect(0, 135, 930, 150)
    ),
    scalable_get_font(SFI_Small)
  );
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_desc_layer, GColorWhite);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Row 1
  int row_x = scalable_x_pp(40, 50);
  int row_y = scalable_y_pp(390, 385);

  const int row_1_text_ico_off = scalable_x_pp(190, 160);
  const int row_1_text_y_off = scalable_y_pp(-40, -20);

  s_remaining_bitmap = bitmaps_get(RESOURCE_ID_REMAINING);
  s_remaining_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_remaining_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_remaining_bmp_layer, s_remaining_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_remaining_bmp_layer));
  s_remaining_layer = util_make_text_layer(
    GRect(row_x + row_1_text_ico_off, row_y + row_1_text_y_off, DISPLAY_W, 100),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += scalable_x_pp(485, 485);

  s_rate_bitmap = bitmaps_get(RESOURCE_ID_RATE);
  s_rate_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_rate_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_rate_bmp_layer, s_rate_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_rate_bmp_layer));
  s_rate_layer = util_make_text_layer(
    GRect(row_x + row_1_text_ico_off, row_y + row_1_text_y_off, DISPLAY_W, 100),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  s_row_1_subtitle_layer = util_make_text_layer(
    GRect(2, row_y + scalable_y_pp(115, 100), DISPLAY_W - ACTION_BAR_W, 40),
    scalable_get_font(SFI_Small)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_row_1_subtitle_layer));

  // Row 2
  row_x = scalable_x(30);
  row_y = scalable_y_pp(665, 670);
  const int row_2_text_ico_off = scalable_x_pp(120, 120);
  const int row_2_text_y_off = scalable_y_pp(-35, -25);
  
  s_last_charge_bitmap = bitmaps_get(RESOURCE_ID_LAST_CHARGE);
  s_last_charge_bmp_layer = bitmap_layer_create(
    GRect(row_x, row_y, ICON_SIZE, ICON_SIZE)
  );
  bitmap_layer_set_compositing_mode(s_last_charge_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_last_charge_bmp_layer, s_last_charge_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_last_charge_bmp_layer));

  const int text_ico_nudge = scalable_x_pp(60, 40);
  s_last_charge_layer = util_make_text_layer(
    GRect(row_x + row_2_text_ico_off + text_ico_nudge, row_y + row_2_text_y_off, DISPLAY_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_last_charge_layer));

  row_x += scalable_x_pp(460, 460);

  s_next_charge_bitmap = bitmaps_get(RESOURCE_ID_NEXT_CHARGE);
  s_next_charge_bmp_layer = bitmap_layer_create(
    GRect(row_x, row_y, ICON_SIZE, ICON_SIZE)
  );
  bitmap_layer_set_compositing_mode(s_next_charge_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_next_charge_bmp_layer, s_next_charge_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_next_charge_bmp_layer));

  s_next_charge_layer = util_make_text_layer(
    GRect(row_x + row_2_text_ico_off, row_y + row_2_text_y_off, DISPLAY_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_next_charge_layer));

  // Row 3
  row_x = scalable_x(10);
  row_y = scalable_y_pp(845, 860);
  const int row_3_text_ico_off = scalable_x_pp(160, 150);
  const int row_3_text_y_off = scalable_y_pp(-30, -20);

  s_battery_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_battery_bmp_layer, GCompOpSet);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_battery_bmp_layer));
  s_battery_layer = util_make_text_layer(
    GRect(row_x + row_3_text_ico_off, row_y + row_3_text_y_off, DISPLAY_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  row_x += scalable_x_pp(430, 450);

  s_reading_bitmap = bitmaps_get(RESOURCE_ID_READING);
  s_reading_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_reading_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_reading_bmp_layer, s_reading_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_reading_bmp_layer));
  const int x_nudge = scalable_x_pp(20, 10);
  s_reading_layer = util_make_text_layer(
    GRect(row_x + row_3_text_ico_off + x_nudge, row_y + row_3_text_y_off, DISPLAY_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Hint for when Muninn is asleep (topmost)
  const GRect hint_rect = scalable_grect_pp(
    GRect(0, 360, 930, 470),
    GRect(0, 355, 930, 475)
  );
  s_hint_layer = util_make_text_layer(hint_rect, scalable_get_font(SFI_Medium));
  text_layer_set_background_color(s_hint_layer, GColorWhite);
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hint_layer, "Hold the Up button  to wake Muninn.");
  layer_add_child(root_layer, text_layer_get_layer(s_hint_layer));

  update_data();

  APP_LOG(APP_LOG_LEVEL_INFO, "Heap free %d", heap_bytes_free());
}

static void window_unload(Window *window) {
  text_layer_destroy(s_status_value_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_row_1_subtitle_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_reading_layer);
  text_layer_destroy(s_remaining_layer);
  text_layer_destroy(s_rate_layer);
  text_layer_destroy(s_hint_layer);
  text_layer_destroy(s_last_charge_layer);
  text_layer_destroy(s_next_charge_layer);

#if !defined(PBL_PLATFORM_APLITE)
  bitmap_layer_destroy(s_mascot_layer);
#endif
  bitmap_layer_destroy(s_battery_bmp_layer);
  bitmap_layer_destroy(s_reading_bmp_layer);
  bitmap_layer_destroy(s_remaining_bmp_layer);
  bitmap_layer_destroy(s_rate_bmp_layer);
  bitmap_layer_destroy(s_last_charge_bmp_layer);
  bitmap_layer_destroy(s_next_charge_bmp_layer);

  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(s_window, click_config_provider);

    s_blink_budget = 5;
  }

  window_stack_push(s_window, true);
}
