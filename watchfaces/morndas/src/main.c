#include <pebble.h>

#include "dict.h"

static Window *s_main_window;
static BitmapLayer *s_bg_layer;
static TextLayer *s_day_layer, *s_wday_layer, *s_time_layer;

static GBitmap *s_bg_bitmap;

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Weekday
  static char s_wday_buffer[16];
  snprintf(s_wday_buffer, sizeof(s_wday_buffer), "%s", dict_get_weekday(tick_time->tm_wday));
  text_layer_set_text(s_wday_layer, s_wday_buffer);

  // Time
  static char s_time_buffer[16];
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  // Day of the month
  static char s_day_buffer[32];
  int day = tick_time->tm_mday;
  if(day == 1 || day == 21 || day == 31) {
    // 1st, 21st, 31st
    snprintf(s_day_buffer, sizeof(s_day_buffer), "%dst of\n%s", day, dict_get_month(tick_time->tm_mon));
  } else if(day == 2 || day == 22) {
    // 2nd, 22nd
    snprintf(s_day_buffer, sizeof(s_day_buffer), "%dnd of\n%s", day, dict_get_month(tick_time->tm_mon));
  } else {
    // nth
    snprintf(s_day_buffer, sizeof(s_day_buffer), "%dth of\n%s", day, dict_get_month(tick_time->tm_mon));
  }
  text_layer_set_text(s_day_layer, s_day_buffer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);

  s_bg_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bg_layer));

  s_wday_layer = text_layer_create(GRect(0, 20, bounds.size.w, 40));
  text_layer_set_text_color(s_wday_layer, GColorWhite);
  text_layer_set_background_color(s_wday_layer, GColorClear);
  text_layer_set_text_alignment(s_wday_layer, GTextAlignmentCenter);
  text_layer_set_font(s_wday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_wday_layer));

  s_time_layer = text_layer_create(GRect(0, 47, bounds.size.w, 60));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_day_layer = text_layer_create(GRect(10, 85, 124, 60));
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_wday_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_day_layer);

  bitmap_layer_destroy(s_bg_layer);
  
  gbitmap_destroy(s_bg_bitmap);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_t now = time(NULL);
  struct tm *time_now = localtime(&now);
  tick_handler(time_now, MINUTE_UNIT);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
