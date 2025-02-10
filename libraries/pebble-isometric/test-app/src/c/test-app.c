#include <pebble.h>

#include <pebble-isometric/pebble-isometric.h>

static Window *s_window;
static Layer *s_canvas_layer;

static void update_proc(Layer *layer, GContext *ctx) {
  // Prepare framebuffer
  isometric_begin(ctx);

  // Draw big box
  isometric_fill_box(Vec3(0, 0, 0), GSize(50, 50), 50, GColorYellow);
  isometric_draw_box(Vec3(0, 0, 0), GSize(50, 50), 50, GColorBlack);

  // Dithered fill test
  isometric_fill_box(Vec3(0, 0, 55), GSize(40, 40), 10, GColorDarkGray);

  // Draw medium box
  isometric_fill_box(Vec3(0, 0, 60), GSize(25, 25), 25, GColorBlue);
  isometric_draw_box(Vec3(0, 0, 60), GSize(25, 25), 25, GColorBlack);

  // Draw small box
  isometric_fill_box(Vec3(0, 0, 85), GSize(10, 10), 10, GColorRed);
  isometric_draw_box(Vec3(0, 0, 85), GSize(10, 10), 10, GColorBlack);

  // Release framebuffer
  isometric_finish(ctx);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  // Set the isometric projection viewport offset
  isometric_set_projection_offset(GPoint(70, 110));
}

static void deinit() {
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
