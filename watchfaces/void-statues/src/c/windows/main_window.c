#include "main_window.h"

// #define TEST

static Window *s_window;
static Layer *s_draw_layer;
static int s_hour_tens, s_hour_units, s_minute_tens, s_minute_units;

static void draw_layer_update_proc(Layer *layer, GContext *ctx) {
  isometric_begin(ctx);
  isometric_set_projection_offset(GPoint(31, (144 / 2) + 1));
  
  // Offsets for B&W shadow rendering
  draw_number(s_hour_tens, GPoint(0, 0));
  draw_number(s_hour_units, GPoint(1, -40));
  draw_number(s_minute_tens, GPoint(64, 45));
  draw_number(s_minute_units, GPoint(64, 5));

  // Finally
  isometric_finish(ctx);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  s_hour_tens = tick_time->tm_hour / 10;
  s_hour_units = tick_time->tm_hour % 10;
  s_minute_tens = tick_time->tm_min / 10;
  s_minute_units = tick_time->tm_min % 10;

#ifdef TEST
  s_hour_tens = 1;
  s_hour_units = 2;
  s_minute_tens = 3;
  s_minute_units = 5;
#endif

  layer_mark_dirty(s_draw_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_draw_layer = layer_create(bounds);
  layer_set_update_proc(s_draw_layer, draw_layer_update_proc);
  layer_add_child(window_layer, s_draw_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_draw_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  // Set the initial time
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  s_hour_tens = tick_time->tm_hour / 10;
  s_hour_units = tick_time->tm_hour % 10;
  s_minute_tens = tick_time->tm_min / 10;
  s_minute_units = tick_time->tm_min % 10;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
