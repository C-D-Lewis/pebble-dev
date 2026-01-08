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

static GBitmap *s_mascot_bitmap;
static AppTimer *s_blink_timer;
static int s_blink_budget;
static bool s_is_blinking, s_animating, s_is_enabled;
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
  time_t wakeup_ts;
  const int wakeup_id = data_get_wakeup_id();
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
  const GRect braid_rect = GRect(0, BRAID_Y, PS_DISP_W - ACTION_BAR_W, BRAID_H);
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
  const int v_div_x = (PS_DISP_W / 2) - scalable_x(40);
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
    GPoint(PS_DISP_W - (ACTION_BAR_W), row_2_div_y)
  );
  // Horizontal below row 2
  const int row_3_div_y = scalable_y_pp(830, 835);
  graphics_draw_line(
    ctx,
    GPoint(0, row_3_div_y),
    GPoint(PS_DISP_W - ACTION_BAR_W, row_3_div_y)
  );
  // Actions BG
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorLightGray));
  GRect actions_rect = GRect(PS_DISP_W - ACTION_BAR_W, 0, ACTION_BAR_W, PS_DISP_H);
  graphics_fill_rect(ctx, actions_rect, 0, GCornerNone);

  const int hint_x = PS_DISP_W - (HINT_W / 2);

  // Enable hint
  const int enable_y = PS_DISP_H / 6 - (HINT_H / 2);
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
  const int menu_y = (PS_DISP_H / 2) - (HINT_H / 2);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx, GRect(hint_x, menu_y, HINT_W, HINT_H),
    3,
    GCornersAll
  );

  // Log hint
  const int log_y = ((5 * PS_DISP_H) / 6) - (HINT_H / 2);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx, GRect(hint_x, log_y, HINT_W, HINT_H),
    3,
    GCornersAll
  );

  // Mascot
  if (s_mascot_bitmap != NULL) {
    bitmaps_destroy(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
  s_mascot_bitmap = bitmaps_get(
    s_is_enabled ? RESOURCE_ID_AWAKE_HEAD : RESOURCE_ID_ASLEEP_HEAD
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    s_mascot_bitmap,
    GRect(scalable_x_pp(40, 60), scalable_y_pp(20, 25), MASCOT_SIZE, MASCOT_SIZE)
  );

  // Blink Muninn's eye
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);
  }

  BatteryChargeState state = battery_state_service_peek();
  GBitmap *batt_state_ptr = bitmaps_get(util_get_battery_resource_id(state.charge_percent));

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_REMAINING),
    GRect(scalable_x(40), scalable_y_pp(390, 385), ICON_SIZE, ICON_SIZE)
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_RATE),
    GRect(scalable_x(530), scalable_y_pp(390, 385), ICON_SIZE, ICON_SIZE)
  );

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_LAST_CHARGE),
    GRect(scalable_x(30), scalable_y_pp(665, 670), ICON_SIZE, ICON_SIZE)
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_NEXT_CHARGE),
    GRect(scalable_x(490), scalable_y_pp(665, 670), ICON_SIZE, ICON_SIZE)
  );

  graphics_draw_bitmap_in_rect(
    ctx,
    batt_state_ptr,
    GRect(scalable_x(10), scalable_y_pp(845, 860), ICON_SIZE, ICON_SIZE)
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_READING),
    GRect(scalable_x_pp(435, 455), scalable_y_pp(850, 860), ICON_SIZE, ICON_SIZE)
  );
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

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_status_layer = util_make_text_layer(
    scalable_grect_pp(
      GRect(0, -35, 1000, 200),
      GRect(0, 5, 1000, 200)
    ),
    scalable_get_font(SFI_LargeBold)
  );
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_status_layer));

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
  int text_ico_off = scalable_x_pp(190, 160);
  int text_y_off = scalable_y_pp(-40, -20);

  s_remaining_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += scalable_x_pp(485, 485);

  s_rate_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  s_row_1_subtitle_layer = util_make_text_layer(
    GRect(2, row_y + scalable_y_pp(115, 100), PS_DISP_W - ACTION_BAR_W, 40),
    scalable_get_font(SFI_Small)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_row_1_subtitle_layer));

  // Row 2
  row_x = scalable_x(30);
  row_y = scalable_y_pp(665, 670);
  text_ico_off = scalable_x_pp(120, 120);
  text_y_off = scalable_y_pp(-35, -25);
  
  const int text_ico_nudge = scalable_x_pp(60, 40);
  s_last_charge_layer = util_make_text_layer(
    GRect(row_x + text_ico_off + text_ico_nudge, row_y + text_y_off, PS_DISP_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_last_charge_layer));

  row_x += scalable_x_pp(460, 460);

  s_next_charge_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_next_charge_layer));

  // Row 3
  row_x = scalable_x(10);
  row_y = scalable_y_pp(845, 860);
  text_ico_off = scalable_x_pp(160, 150);
  text_y_off = scalable_y_pp(-30, -20);

  s_battery_layer = util_make_text_layer(
    GRect(row_x + text_ico_off, row_y + text_y_off, PS_DISP_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  row_x += scalable_x_pp(430, 450);

  const int x_nudge = scalable_x_pp(20, 10);
  s_reading_layer = util_make_text_layer(
    GRect(row_x + text_ico_off + x_nudge, row_y + text_y_off, PS_DISP_W, 100),
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

  APP_LOG(APP_LOG_LEVEL_INFO, "Heap %d", heap_bytes_free());
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

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
    window_set_click_config_provider(s_window, click_config_provider);

    s_blink_budget = 5;
  }

  window_stack_push(s_window, true);
}
