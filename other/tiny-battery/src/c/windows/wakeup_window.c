#include "wakeup_window.h"

static Window *s_window;
static TextLayer *s_summary_layer;

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  // Current battery level
  s_summary_layer = util_make_text_layer(GRect(10, 20, 144, 100), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_summary_layer, "Sample taken!");
  layer_add_child(root_layer, text_layer_get_layer(s_summary_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_summary_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void timer_callback(void *data) {
  window_stack_pop_all(true);
}

void wakeup_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  // Pop after a while (progressbar?)
  app_timer_register(3000, timer_callback, NULL);
  vibes_double_pulse();
}
