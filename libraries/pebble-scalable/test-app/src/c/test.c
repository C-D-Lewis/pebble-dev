#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  FontId_Small = 0,
  FontId_Medium,
} FontId;

static Window *s_window;
static Layer *s_canvas_layer;

static GFont s_gothic_18, s_gothic_24;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);

  // Half width, 2% height, specific position
  graphics_fill_rect(
    ctx,
    GRect(0, 10, scalable_x(500), scalable_y(20)),
    0,
    GCornerNone
  );

  // Half height, 2% width, specific position
  graphics_fill_rect(
    ctx,
    GRect(10, 0, scalable_x(20), scalable_y(500)),
    0,
    GCornerNone
  );

  // 12% size and position
  graphics_fill_rect(ctx, scalable_grect(120, 120, 120, 120), 0, GCornerNone);

  // Nudge just inside the 12% rect
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(
    ctx,
    scalable_nudge_xy(scalable_grect(110, 110, 10, 10), 3, 3),
    0,
    GCornerNone
  );

  // Text in the middle third
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    "This text should appear in the middle third on any platform or screen size",
    scalable_get_font(FontId_Small),
    scalable_grect(0, 330, 1000, 330),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentCenter,
    NULL
  );

  // Center a GRect horizontally
  const GRect centered_h = scalable_center_x(scalable_grect(100, 800, 400, 50));
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, centered_h, 0, GCornerNone);

  // Center third outline
  graphics_draw_rect(ctx, scalable_center(scalable_grect(10, 10, 330, 330)));

  // Text at a negative Y
  graphics_draw_text(
    ctx,
    "Off the top",
    scalable_get_font(FontId_Small),
    scalable_grect(0, -60, 1000, 330),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentCenter,
    NULL
  );
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
  s_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);

  // The small font - regular screens use Gothic 18, Emery uses Gothic 24
  scalable_set_fonts(FontId_Small, &s_gothic_18, NULL, &s_gothic_24);

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
