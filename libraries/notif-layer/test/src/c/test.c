#include <pebble.h>
#include "./notif_layer.h"

static Window *s_window;
static TextLayer *s_text_layer;
static NotifLayer *s_notif_layer;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  notif_layer_show(s_notif_layer, "You pressed the select button!");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Press a button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  s_notif_layer = notif_layer_create();
  notif_layer_add_to_window(s_notif_layer, window);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  notif_layer_destroy(s_notif_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, click_config_provider);
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
