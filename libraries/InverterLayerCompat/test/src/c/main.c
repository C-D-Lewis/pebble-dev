#include <pebble.h>
#include "./InverterLayerCompat.h"

static Window *s_window;
static TextLayer *s_text_layer;
static InverterLayerCompat *s_inverter_layer;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "I should be inverted");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  GColor replacement = PBL_IF_COLOR_ELSE(GColorBlue, GColorWhite);
  s_inverter_layer = inverter_layer_compat_create(grect_inset(bounds, GEdgeInsets(20)), GColorBlack, replacement);
  layer_add_child(window_layer, inverter_layer_compat_get_layer(s_inverter_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  inverter_layer_compat_destroy(s_inverter_layer);

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
