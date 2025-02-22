#include "main_window.h"

#define X_ROOT PBL_IF_ROUND_ELSE(65, 47)
#define Y_ROOT PBL_IF_ROUND_ELSE(40, 34)
#define GRECT_HOUR_LAYER GRect(X_ROOT, Y_ROOT, 50, 100)

static Window *s_window;
static BitmapLayer *s_hour_layer;

static GBitmap *s_hour_bitmap;

static int get_resource_id(int hours) {
  switch(hours) {
    case 0:
      return RESOURCE_ID_TUNGSTEN_0;
    case 1:
      return RESOURCE_ID_TUNGSTEN_1;
    case 2:
      return RESOURCE_ID_TUNGSTEN_2;
    case 3:
      return RESOURCE_ID_TUNGSTEN_3;
    case 4:
      return RESOURCE_ID_TUNGSTEN_4;
    case 5:
      return RESOURCE_ID_TUNGSTEN_5;
    case 6:
      return RESOURCE_ID_TUNGSTEN_6;
    case 7:
      return RESOURCE_ID_TUNGSTEN_7;
    case 8:
      return RESOURCE_ID_TUNGSTEN_8;
    case 9:
      return RESOURCE_ID_TUNGSTEN_9;
    default:
      return RESOURCE_ID_TUNGSTEN_0;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  gbitmap_destroy(s_hour_bitmap);

  const int hours = tick_time->tm_hour;
  s_hour_bitmap = gbitmap_create_with_resource(get_resource_id(hours));
  bitmap_layer_set_bitmap(s_hour_layer, s_hour_bitmap);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = bitmap_layer_create(GRECT_HOUR_LAYER);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_hour_layer));
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_hour_layer);
  gbitmap_destroy(s_hour_bitmap);

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

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
