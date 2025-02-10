#include <pebble.h>

#include <pebble-dash-api/pebble-dash-api.h>
#include <pebble-events/pebble-events.h>

#define APP_NAME "Dual Gauge"
#define NO_VALUE -1
// #define TEST
// #define TEST_FULL

static Window *s_window;
static Layer *s_canvas_layer;

static GBitmap *s_watch_bitmap, *s_phone_bitmap;
static int s_local_perc, s_remote_perc = NO_VALUE;
static char s_time_buff[8];

static void get_data_callback(DataType type, DataValue value) {
  s_remote_perc = value.integer_value;
  layer_mark_dirty(s_canvas_layer);
}

static void error_callback(ErrorCode code) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "ErrorCode: %d", code);

  if(code != ErrorCodeSuccess) {
    // Not available
    s_remote_perc = NO_VALUE;
    layer_mark_dirty(s_canvas_layer);
  }
}

static void update_gauges() {
  BatteryChargeState state = battery_state_service_peek();
  s_local_perc = (int)state.charge_percent;

  dash_api_get_data(DataTypeBatteryPercent, get_data_callback);
  layer_mark_dirty(s_canvas_layer);

#if defined(TEST)
  dash_api_fake_get_data_response(DataTypeBatteryPercent, 75, NULL);
#endif
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_unobstructed_bounds(layer);

  int unob_gap = bounds.size.h / 3;

  GPoint cursor = GPointZero;
  cursor.y += unob_gap;

  // Time
  cursor.y -= 35;
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, s_time_buff, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), 
    GRect(cursor.x, cursor.y, bounds.size.w, bounds.size.h),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  // Icons
  cursor.y += unob_gap;
  cursor.y += 10;
  cursor.x = PBL_IF_ROUND_ELSE(43, 24);

  GRect bitmap_rect = GRect(cursor.x, cursor.y, 40, 40);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_watch_bitmap, bitmap_rect);

  bitmap_rect.origin.x += PBL_IF_ROUND_ELSE(56, 57);
  graphics_draw_bitmap_in_rect(ctx, s_phone_bitmap, bitmap_rect);

#if defined(TEST_FULL)
  s_local_perc = 78;
  s_remote_perc = 100;
#endif

  cursor.x = PBL_IF_ROUND_ELSE(50, 31);
  if(s_local_perc == 100) {
    cursor.x -= 5;
  }

  // local 
  cursor.y += 40;
  static char s_local_buff[8];
  snprintf(s_local_buff, sizeof(s_local_buff), "%d", s_local_perc);
  graphics_draw_text(ctx, s_local_buff, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS), 
    GRect(cursor.x, cursor.y, bounds.size.w, bounds.size.h),
    GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  if(s_local_perc == 100) {
    cursor.x += 5;
  }

  // remote
  cursor.x += PBL_IF_ROUND_ELSE(57, 58);
  static char s_remote_buff[8];
  snprintf(s_remote_buff, sizeof(s_remote_buff), (s_remote_perc == NO_VALUE) ? "-" : "%d", s_remote_perc);
  graphics_draw_text(ctx, s_remote_buff, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS), 
    GRect(cursor.x, cursor.y, bounds.size.w, bounds.size.h),
    GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  strftime(s_time_buff, sizeof(s_time_buff), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  if(tick_time->tm_min % 15 == 0) {
    update_gauges();
  }

  layer_mark_dirty(s_canvas_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_watch_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_WATCH);
  s_phone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_PHONE);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
  
  update_gauges();
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  gbitmap_destroy(s_watch_bitmap);
  gbitmap_destroy(s_phone_bitmap);

  window_destroy(s_window);
}

void main_window_push() {
  dash_api_init(APP_NAME, error_callback);
  events_app_message_open();

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  time_t now = time(NULL);
  struct tm *time_now = localtime(&now);
  tick_handler(time_now, MINUTE_UNIT);
}
