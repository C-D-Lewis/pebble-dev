#include "main_window.h"

static Window *s_window;
static BitmapLayer *s_bg_layer;
static TextLayer *s_time_layer;

static GBitmap *s_bg_bitmap;

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  static char time_buffer[8];
  strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
  text_layer_set_text(s_time_layer, time_buffer);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  
  s_bg_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_bg_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_bg_layer));

  s_time_layer = text_layer_create(GRect(0, 20, 200, 100));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(
    s_time_layer,
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ABSENDER_64))
  );
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_bg_layer);
  text_layer_destroy(s_time_layer);

  gbitmap_destroy(s_bg_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_background_color(s_window, GColorBlack);
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, MINUTE_UNIT);
}
