#include <pebble.h>
#include <pebble-layout/pebble-layout.h>

static Window *s_window;
static Layer *s_canvas_layer;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);

  // Half width, 2% height
  int w = pl_x(50);
  int h = pl_y(2);
  graphics_fill_rect(ctx, GRect(0, 10, w, h), 0, GCornerNone);

  // 10% size and position
  GRect ten_perc_rect = pl_grect(10, 10, 10, 10);
  graphics_fill_rect(ctx, ten_perc_rect, 0, GCornerNone);

  // Nudge just inside the 10% rect
  GRect point = pl_grect(9, 9, 1, 1);
  GRect inside_rect = pl_nudge_xy(point, 2, 2);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, inside_rect, 0, GCornerNone);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
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
