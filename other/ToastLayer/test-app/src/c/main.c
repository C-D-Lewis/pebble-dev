#include <pebble.h>
#include "./ToastLayer.h"

static Window *s_window;
static TextLayer *s_text_layer;
static ToastLayer *s_toast_layer;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "I should be inverted");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  s_toast_layer = toast_layer_create(window);
  toast_layer_show(s_toast_layer, "Hello, this is a ToastLayer!", 3000);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  toast_layer_destroy(s_toast_layer);

  window_destroy(s_window);
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
