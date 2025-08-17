#include "splash_window.h"

#define INTERVAL_MS 150  // Animation frame interval

static Window *s_window;
static BitmapLayer *s_logo_layer;

static GBitmap *s_logo_bitmap;

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);

  s_logo_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  bitmap_layer_set_compositing_mode(s_logo_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_logo_layer);
  gbitmap_destroy(s_logo_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

void splash_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void splash_window_remove_from_stack() {
  if(s_window) {
    window_stack_remove(s_window, true);
  }
}
