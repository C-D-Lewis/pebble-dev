#include "main_window.h"

#define Y_ROOT PBL_IF_ROUND_ELSE(60, 50)
#define GRECT_HOUR_LAYER GRect(0, Y_ROOT, PBL_IF_ROUND_ELSE(180, 144), 50)

static Window *s_window;
static TextLayer *s_hour_layer;

/**
 * Handler when a tick occurs.
 */
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  static char time_buffer[8];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  text_layer_set_text(s_hour_layer, time_buffer);
}

/**
 * Window load event handler.
 */
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = text_layer_create(GRECT_HOUR_LAYER);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  GFont tungsten_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TUNGSTEN_48));
  text_layer_set_font(s_hour_layer, tungsten_font);
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
}

/**
 * Window unload event handler.
 */
static void window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);

  window_destroy(s_window);
  s_window = NULL;
}

/**
 * Load the main window and push it to the stack.
 */
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
