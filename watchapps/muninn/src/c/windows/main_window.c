#include "main_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define ACTION_BAR_W 18
  #define FONT_KEY_XS FONT_KEY_GOTHIC_18
  #define FONT_KEY_S FONT_KEY_GOTHIC_24
  #define FONT_KEY_M FONT_KEY_GOTHIC_28
  #define FONT_KEY_L_B FONT_KEY_GOTHIC_28_BOLD
  #define MASCOT_RECT GRect(20, 8, 48, 48)
  #define STATUS_LABEL_RECT GRect(95, 3, DISPLAY_W, 100)
  #define STATUS_VALUE_RECT GRect(93, 22, DISPLAY_W - 75, 100)
  #define EYE_RECT GRect(58, 12, 4, 4)
  #define BRAID_Y 85
  #define DESC_RECT GRect(10, 57, DISPLAY_W - ACTION_BAR_W - 20, 100)
  #define ROW_1_X 18
  #define ROW_1_Y 115
  #define ROW_2_X 10
  #define ROW_2_Y 192
  #define ROW_1_GAP 90
  #define ROW_2_GAP 94
  #define ROW_1_SUBTITLE "    Days left        Est. %/day"
  #define HINT_W 20
  #define HINT_H 48
  #define ROW_DIV_Y 178
  #define ROW_DIV_X (DISPLAY_W / 2) - 8
  #define ROW_DIV_H 80
  #define ROW_2_TEXT_Y_OFF 7
  #define HOLD_RAD 5
  #define MENU_BG_COLOR GColorJazzberryJam
#else
  #define ACTION_BAR_W 12
  #define FONT_KEY_XS FONT_KEY_GOTHIC_14
  #define FONT_KEY_S FONT_KEY_GOTHIC_18
  #define FONT_KEY_M FONT_KEY_GOTHIC_24
  #define FONT_KEY_L_B FONT_KEY_GOTHIC_28_BOLD
  #define MASCOT_RECT GRect(3, 3, 48, 48)
  #define STATUS_LABEL_RECT GRect(60, 0, DISPLAY_W, 100)
  #define STATUS_VALUE_RECT GRect(58, 13, DISPLAY_W - 75, 100)
  #define EYE_RECT GRect(41, 7, 4, 4)
  #define BRAID_Y 68
  #define DESC_RECT GRect(5, 47, DISPLAY_W - ACTION_BAR_W - 10, 100)
  #define ROW_1_X 2
  #define ROW_1_Y 92
  #define ROW_2_X 2
  #define ROW_2_Y 142
  #define ROW_1_GAP 60
  #define ROW_2_GAP 67
  #define ROW_1_SUBTITLE "  Days left     Est. %/day"
  #define HINT_W 14
  #define HINT_H 38
  #define ROW_DIV_Y 137
  #define ROW_DIV_X (DISPLAY_W / 2) - 14
  #define ROW_DIV_H 55
  #define ROW_2_TEXT_Y_OFF 5
  #define HOLD_RAD 3
  #define MENU_BG_COLOR PBL_IF_COLOR_ELSE(GColorJazzberryJam, GColorLightGray)
#endif

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
  *s_status_label_layer,
  *s_status_value_layer,
  *s_desc_layer,
  *s_battery_layer,
  *s_reading_layer,
  *s_row_1_subtitle_layer,
  *s_remaining_layer,
  *s_rate_layer,
  *s_hint_layer;
static BitmapLayer
  *s_mascot_layer,
  *s_battery_bmp_layer,
  *s_reading_bmp_layer,
  *s_remaining_bmp_layer,
  *s_rate_bmp_layer;
static GBitmap
  *s_mascot_bitmap,
  *s_battery_bitmap,
  *s_reading_bitmap,
  *s_remaining_bitmap,
  *s_rate_bitmap,
  *s_braid_bitmap;

static AppTimer *s_blink_timer;
static bool s_is_blinking;
static int s_blink_budget;

static void schedule_blink();

static uint32_t get_battery_resource_id(int charge_percent) {
  if (charge_percent > 66) return RESOURCE_ID_BATTERY_HIGH;
  if (charge_percent > 33) return RESOURCE_ID_BATTERY_MEDIUM;
  return RESOURCE_ID_BATTERY_LOW;
}

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
  if (s_blink_budget == 0) return;

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

static void update_data() {
  if (s_mascot_bitmap) {
    gbitmap_destroy(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
  if (s_battery_bitmap) {
    gbitmap_destroy(s_battery_bitmap);
    s_battery_bitmap = NULL;
  }

  time_t wakeup_ts;
  const int wakeup_id = data_get_wakeup_id();
  const bool is_enabled = wakeup_id != DATA_EMPTY;
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);
  if (is_enabled && !found) {
    // Something went wrong
    data_set_error("Scheduled wakeup was not found");
  }
  
  s_mascot_bitmap = gbitmap_create_with_resource(
    is_enabled ? RESOURCE_ID_AWAKE : RESOURCE_ID_ASLEEP
  );
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");

  cancel_blink();

  // Battery now
  BatteryChargeState state = battery_state_service_peek();
  const int charge_percent = state.charge_percent;
  s_battery_bitmap = gbitmap_create_with_resource(get_battery_resource_id(charge_percent));
  bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
  static char s_battery_buff[16];
  snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buff);

  // Sometimes
  if (!is_enabled) {
    text_layer_set_text(s_desc_layer, "Not monitoring");
    text_layer_set_text(s_reading_layer, "-");
    text_layer_set_text(s_remaining_layer, "-");
    text_layer_set_text(s_rate_layer, "-");

    layer_set_hidden(text_layer_get_layer(s_hint_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_hint_layer), true);
    schedule_blink();

    text_layer_set_text(s_desc_layer, "Passively monitoring");

    // Days remaining
    const int days_remaining = data_calculate_days_remaining();
    if (days_remaining == DATA_EMPTY) {
      text_layer_set_text(s_remaining_layer, "-");
    } else {
      static char s_remaining_buff[16];
      snprintf(s_remaining_buff, sizeof(s_remaining_buff), "%d", days_remaining);
      text_layer_set_text(s_remaining_layer, s_remaining_buff);
    }

    // Rate per day
    const int rate = data_calculate_avg_discharge_rate();
    if (rate == DATA_EMPTY) {
      text_layer_set_text(s_rate_layer, "-");
    } else {
      static char s_rate_buff[16];
      snprintf(s_rate_buff, sizeof(s_rate_buff), "%d%%", rate);
      text_layer_set_text(s_rate_layer, s_rate_buff);
    }

    // Next reading
    static char s_wakeup_buff[8];
    util_fmt_time_ago(wakeup_ts, &s_wakeup_buff[0], sizeof(s_wakeup_buff));
    text_layer_set_text(s_reading_layer, s_wakeup_buff);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Divider braid
  const GRect braid_rect = GRect(0, BRAID_Y, DISPLAY_W, 14);
  graphics_draw_bitmap_in_rect(ctx, s_braid_bitmap, braid_rect);

  // Actions BG
  graphics_context_set_fill_color(ctx, MENU_BG_COLOR);
  GRect actions_rect = GRect(DISPLAY_W - ACTION_BAR_W, 0, ACTION_BAR_W, DISPLAY_H);
  graphics_fill_rect(ctx, actions_rect, 0, GCornerNone);

  int hint_x = DISPLAY_W - (HINT_W / 2);

  // Enable hint
  int enable_y = DISPLAY_H / 6 - (HINT_H / 2);
  GRect enable_rect = GRect(hint_x, enable_y, HINT_W, HINT_H);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, enable_rect, 3, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorWhite);
  GPoint select_center = {
    .x = hint_x + (HINT_W / 2),
    .y = enable_y + (HINT_H / 2)
  };
  graphics_fill_circle(ctx, select_center, HOLD_RAD);

  // Blink Muninn's eye
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);  
  }

  // Menu hint
  int menu_y = (DISPLAY_H / 2) - (HINT_H / 2);
  GRect menu_rect = GRect(hint_x, menu_y, HINT_W, HINT_H);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, menu_rect, 3, GCornersAll);

  // Row divider
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(
    ctx,
    GPoint(0, ROW_DIV_Y),
    GPoint(DISPLAY_W - (1 * ACTION_BAR_W), ROW_DIV_Y)
  );
  graphics_draw_line(
    ctx,
    GPoint(ROW_DIV_X, ROW_DIV_Y),
    GPoint(ROW_DIV_X, ROW_DIV_Y - ROW_DIV_H)
  );
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  const bool should_enable = data_get_wakeup_id() == DATA_EMPTY;
  if (should_enable) {
    s_blink_budget = 5;
    schedule_blink();

    data_activation_update();
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

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  stat_window_push();
}

static void click_config_provider(void *context) {
  window_long_click_subscribe(BUTTON_ID_UP, 1000, up_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);

  // Debugging info
  window_long_click_subscribe(BUTTON_ID_DOWN, 3000, down_long_click_handler, NULL);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  GFont font_xs = fonts_get_system_font(FONT_KEY_XS);
  GFont font_s = fonts_get_system_font(FONT_KEY_S);
  GFont font_m = fonts_get_system_font(FONT_KEY_M);
  GFont font_l_b = fonts_get_system_font(FONT_KEY_L_B);

  s_braid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRAID);

  // Mascot
  s_mascot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_AWAKE);
  s_mascot_layer = bitmap_layer_create(MASCOT_RECT);
  bitmap_layer_set_compositing_mode(s_mascot_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_mascot_layer));

  // On top of mascot
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_status_label_layer = util_make_text_layer(STATUS_LABEL_RECT, font_s);
  text_layer_set_text(s_status_label_layer, "Muninn is");
  layer_add_child(root_layer, text_layer_get_layer(s_status_label_layer));

  s_status_value_layer = util_make_text_layer(STATUS_VALUE_RECT, font_l_b);
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  s_desc_layer = util_make_text_layer(DESC_RECT, font_xs);
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Top row
  const int text_offset = 27;
  int row_x = ROW_1_X;
  int row_y = ROW_1_Y;

  s_remaining_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REMAINING);
  s_remaining_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_remaining_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_remaining_bmp_layer, s_remaining_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_remaining_bmp_layer));
  s_remaining_layer = util_make_text_layer(
    GRect(row_x + text_offset, row_y - 7, DISPLAY_W, 100),
    font_l_b
  );
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += ROW_1_GAP;

  s_rate_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RATE);
  s_rate_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_rate_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_rate_bmp_layer, s_rate_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_rate_bmp_layer));
  s_rate_layer = util_make_text_layer(
    GRect(row_x + text_offset, row_y - 7, DISPLAY_W, 100),
    font_l_b
  );
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  s_row_1_subtitle_layer = util_make_text_layer(
    GRect(1, row_y + 22, DISPLAY_W - ACTION_BAR_W, 40),
#if defined(PBL_PLATFORM_EMERY)
    font_s
#else
    font_xs
#endif
  );
  text_layer_set_text(s_row_1_subtitle_layer, ROW_1_SUBTITLE);
  layer_add_child(root_layer, text_layer_get_layer(s_row_1_subtitle_layer));

  // Bottom row
  row_x = ROW_2_X;
  row_y = ROW_2_Y;

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_HIGH);
  s_battery_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_battery_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_battery_bmp_layer));
  s_battery_layer = util_make_text_layer(
    GRect(row_x + text_offset - 2, row_y - ROW_2_TEXT_Y_OFF, DISPLAY_W, 100),
    font_m
  );
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  row_x += ROW_2_GAP;

  s_reading_bitmap = gbitmap_create_with_resource(RESOURCE_ID_READING);
  s_reading_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_reading_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_reading_bmp_layer, s_reading_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_reading_bmp_layer));
  s_reading_layer = util_make_text_layer(
    GRect(row_x + text_offset, row_y - ROW_2_TEXT_Y_OFF, DISPLAY_W, 100),
    font_m
  );
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Hint for when Muninn is asleep
  const GRect hint_rect = GRect(
    8,
    BRAID_Y + 14,
    DISPLAY_W - ACTION_BAR_W - 16,
    (ROW_DIV_Y - BRAID_Y - 14)
  );
  s_hint_layer = util_make_text_layer(hint_rect, font_s);
  text_layer_set_background_color(s_hint_layer, GColorWhite);
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hint_layer, "Wake Muninn to begin monitoring.");
  layer_add_child(root_layer, text_layer_get_layer(s_hint_layer));

  update_data();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_status_label_layer);
  text_layer_destroy(s_status_value_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_row_1_subtitle_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_reading_layer);
  text_layer_destroy(s_remaining_layer);
  text_layer_destroy(s_rate_layer);
  text_layer_destroy(s_hint_layer);

  gbitmap_destroy(s_mascot_bitmap);
  gbitmap_destroy(s_braid_bitmap);
  gbitmap_destroy(s_battery_bitmap);
  gbitmap_destroy(s_reading_bitmap);
  gbitmap_destroy(s_remaining_bitmap);
  gbitmap_destroy(s_rate_bitmap);

  bitmap_layer_destroy(s_mascot_layer);
  bitmap_layer_destroy(s_battery_bmp_layer);
  bitmap_layer_destroy(s_reading_bmp_layer);
  bitmap_layer_destroy(s_remaining_bmp_layer);
  bitmap_layer_destroy(s_rate_bmp_layer);

  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_data();
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(s_window, click_config_provider);

    // Update stuff while app is open
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    s_blink_budget = 5;
  }

  window_stack_push(s_window, true);
}
