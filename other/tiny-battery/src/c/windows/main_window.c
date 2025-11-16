#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
  *s_status_label_layer,
  *s_status_value_layer,
  *s_desc_layer,
  *s_started_layer,
  *s_reading_layer,
  *s_remaining_layer,
  *s_rate_layer;
static BitmapLayer
  *s_mascot_layer,
  *s_started_bmp_layer,
  *s_reading_bmp_layer,
  *s_remaining_bmp_layer,
  *s_rate_bmp_layer;
static GBitmap
  *s_mascot_bitmap,
  *s_started_bitmap,
  *s_reading_bitmap,
  *s_remaining_bitmap,
  *s_rate_bitmap,
  *s_braid_bitmap;

static void update_data() {
  gbitmap_destroy(s_mascot_bitmap);

  bool is_enabled = data_get_wakeup_id() != DATA_EMPTY;
  s_mascot_bitmap = gbitmap_create_with_resource(is_enabled ? RESOURCE_ID_AWAKE : RESOURCE_ID_ASLEEP);

  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");

  if (!is_enabled) {
    text_layer_set_text(s_desc_layer, "Not monitoring");
    text_layer_set_text(s_started_layer, "-");
    text_layer_set_text(s_reading_layer, "-");
    text_layer_set_text(s_remaining_layer, "-");
    text_layer_set_text(s_rate_layer, "-");
  } else {
    text_layer_set_text(s_desc_layer, "Passively monitoring");

    // Started
    static char s_started_buff[16];
    util_fmt_time_ago(data_get_discharge_start_time(), &s_started_buff[0], sizeof(s_started_buff));
    text_layer_set_text(s_started_layer, s_started_buff);

    // Next reading
    time_t wakeup_ts;
    wakeup_query(data_get_wakeup_id(), &wakeup_ts);

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
      snprintf(s_rate_buff, sizeof(s_rate_buff), "%d%%/d", rate);
      text_layer_set_text(s_rate_layer, s_rate_buff);
    }
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Togggle enable hint
  int hint_w = 16;
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
    data_prepare();
    data_initial_sample();
    wakeup_schedule_next();

    vibes_long_pulse();
  } else {
    wakeup_unschedule();
    vibes_long_pulse();
  }

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

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  // Mascot
  s_mascot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_AWAKE);
  s_mascot_layer = bitmap_layer_create(GRect(0, 0, 64, 64));
  bitmap_layer_set_compositing_mode(s_mascot_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_mascot_layer));

  s_status_label_layer = util_make_text_layer(GRect(67, 5, WIDTH - 75, 100), sys_18);
  text_layer_set_text(s_status_label_layer, "Muninn is");
  layer_add_child(root_layer, text_layer_get_layer(s_status_label_layer));

  s_status_value_layer = util_make_text_layer(GRect(66, 18, WIDTH - 75, 100), sys_28_bold);
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  s_desc_layer = util_make_text_layer(GRect(5, 62, WIDTH - 10, 100), sys_18);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Row 1
  int row_x = 5;
  int row_y = 110;

  s_started_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STARTED);
  s_started_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_started_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_started_bmp_layer, s_started_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_started_bmp_layer));
  s_started_layer = util_make_text_layer(GRect(row_x + 25, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_started_layer));

  row_x += 65;

  s_reading_bitmap = gbitmap_create_with_resource(RESOURCE_ID_READING);
  s_reading_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_reading_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_reading_bmp_layer, s_reading_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_reading_bmp_layer));
  s_reading_layer = util_make_text_layer(GRect(row_x + 27, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Row 2
  row_x = 5;
  row_y += 28;

  s_remaining_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REMAINING);
  s_remaining_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_remaining_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_remaining_bmp_layer, s_remaining_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_remaining_bmp_layer));
  s_remaining_layer = util_make_text_layer(GRect(row_x + 27, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += 65;

  s_rate_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RATE);
  s_rate_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, 24, 24));
  bitmap_layer_set_compositing_mode(s_rate_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_rate_bmp_layer, s_rate_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_rate_bmp_layer));
  s_rate_layer = util_make_text_layer(GRect(row_x + 27, row_y - 5, WIDTH, 100), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  update_data();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_status_label_layer);
  text_layer_destroy(s_status_value_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_started_layer);
  text_layer_destroy(s_reading_layer);
  text_layer_destroy(s_remaining_layer);
  text_layer_destroy(s_rate_layer);

  gbitmap_destroy(s_mascot_bitmap);
  gbitmap_destroy(s_braid_bitmap);
  gbitmap_destroy(s_started_bitmap);
  gbitmap_destroy(s_reading_bitmap);
  gbitmap_destroy(s_remaining_bitmap);
  gbitmap_destroy(s_rate_bitmap);

  bitmap_layer_destroy(s_mascot_layer);
  bitmap_layer_destroy(s_started_bmp_layer);
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
  }

  window_stack_push(s_window, true);
}
