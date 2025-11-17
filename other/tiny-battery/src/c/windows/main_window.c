#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
  *s_status_label_layer,
  *s_status_value_layer,
  *s_desc_layer,
  *s_battery_layer,
  *s_reading_layer,
  *s_remaining_layer,
  *s_rate_layer;
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
  
  s_mascot_bitmap = gbitmap_create_with_resource(is_enabled ? RESOURCE_ID_AWAKE : RESOURCE_ID_ASLEEP);
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");

  cancel_blink();

  if (!is_enabled) {
    text_layer_set_text(s_desc_layer, "Not monitoring");
    text_layer_set_text(s_battery_layer, "-");
    text_layer_set_text(s_reading_layer, "-");
    text_layer_set_text(s_remaining_layer, "-");
    text_layer_set_text(s_rate_layer, "-");

    s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY);
    bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);

  } else {
    schedule_blink();

    text_layer_set_text(s_desc_layer, "Passively monitoring");

    // Battery now
    BatteryChargeState state = battery_state_service_peek();
    const int charge_percent = state.charge_percent;
    s_battery_bitmap = gbitmap_create_with_resource(get_battery_resource_id(charge_percent));
    bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
    static char s_battery_buff[16];
    snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", charge_percent);
    text_layer_set_text(s_battery_layer, s_battery_buff);

    // Next reading
    static char s_wakeup_buff[8];
    util_fmt_time_ago(wakeup_ts, &s_wakeup_buff[0], sizeof(s_wakeup_buff));
    text_layer_set_text(s_reading_layer, s_wakeup_buff);

    // Days remaining
    const int days_remaining = data_calculate_days_remaining();
    if (days_remaining == DATA_EMPTY) {
      text_layer_set_text(s_remaining_layer, "-");
    } else {
      static char s_remaining_buff[16];
      snprintf(s_remaining_buff, sizeof(s_remaining_buff), "%d d", days_remaining);
      text_layer_set_text(s_remaining_layer, s_remaining_buff);
    }

    // Rate per day
    const int rate = data_get_history_avg_rate();
    if (rate == DATA_EMPTY) {
      text_layer_set_text(s_rate_layer, "-");
    } else {
      static char s_rate_buff[16];
      snprintf(s_rate_buff, sizeof(s_rate_buff), "%d%%", rate);
      text_layer_set_text(s_rate_layer, s_rate_buff);
    }
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Togggle enable hint
  int hint_w = 14;
  int hint_h = 32;
  int hint_x = WIDTH - (hint_w / 2);
  int select_y = ((HEIGHT - hint_h) / 2) - 16;
  GRect select_rect = GRect(hint_x, select_y, hint_w, hint_h);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, select_rect, 3, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorWhite);
  GPoint select_center = {
    .x = hint_x + (hint_w / 2),
    .y = select_y + (hint_h / 2)
  };
  graphics_fill_circle(ctx, select_center, 3);

  // Blink?
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, GColorBlack);

    const GRect eye_rect = GRect(43, 13, 4, 4);
    graphics_fill_rect(ctx, eye_rect, 0, GCornerNone);  
  }

  // Divider
  const GRect braid_rect = GRect(0, 90, WIDTH, 14);
  graphics_draw_bitmap_in_rect(ctx, s_braid_bitmap, braid_rect);

  // Menu hint
  int menu_y = HEIGHT - hint_h - 17;
  GRect menu_rect = GRect(hint_x, menu_y, hint_w, hint_h);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, menu_rect, 3, GCornersAll);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool should_enable = data_get_wakeup_id() == DATA_EMPTY;

  if (should_enable) {
    s_blink_budget = 5;
    schedule_blink();

    data_initial_sample();
    wakeup_schedule_next();
  } else {
    wakeup_unschedule();
  }

  vibes_long_pulse();
  update_data();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  menu_window_push();
}

static void click_config_provider(void *context) {
  window_long_click_subscribe(BUTTON_ID_SELECT, 1000, select_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  GFont sys_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GFont sys_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  GFont sys_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  s_braid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRAID);

  // Mascot
  s_mascot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_AWAKE);
  s_mascot_layer = bitmap_layer_create(GRect(0, 0, 64, 64));
  bitmap_layer_set_compositing_mode(s_mascot_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_mascot_layer));

  // On top of mascot
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_status_label_layer = util_make_text_layer(GRect(67, 5, WIDTH - 75, 100), sys_18);
  text_layer_set_text(s_status_label_layer, "Muninn is");
  layer_add_child(root_layer, text_layer_get_layer(s_status_label_layer));

  s_status_value_layer = util_make_text_layer(GRect(66, 18, WIDTH - 75, 100), sys_28_bold);
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  s_desc_layer = util_make_text_layer(GRect(5, 64, WIDTH - 10, 100), sys_18);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Row 1
  const int text_offset = 25;
  int row_x = 5;
  int row_y = 110;

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_HIGH);
  s_battery_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_battery_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_battery_bmp_layer));
  s_battery_layer = util_make_text_layer(GRect(row_x + text_offset, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  row_x += 65;

  s_reading_bitmap = gbitmap_create_with_resource(RESOURCE_ID_READING);
  s_reading_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_reading_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_reading_bmp_layer, s_reading_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_reading_bmp_layer));
  s_reading_layer = util_make_text_layer(GRect(row_x + text_offset, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Row 2
  row_x = 5;
  row_y += 28;

  s_remaining_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REMAINING);
  s_remaining_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_remaining_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_remaining_bmp_layer, s_remaining_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_remaining_bmp_layer));
  s_remaining_layer = util_make_text_layer(GRect(row_x + text_offset, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += 65;

  s_rate_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RATE);
  s_rate_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_rate_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_rate_bmp_layer, s_rate_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_rate_bmp_layer));
  s_rate_layer = util_make_text_layer(GRect(row_x + text_offset, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  update_data();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_status_label_layer);
  text_layer_destroy(s_status_value_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_reading_layer);
  text_layer_destroy(s_remaining_layer);
  text_layer_destroy(s_rate_layer);

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
