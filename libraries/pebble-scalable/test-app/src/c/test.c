#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  FontId_Small = 0,
  FontId_Medium,
} FontId;

static Window *s_window;
static Layer *s_canvas_layer;
static GBitmap *s_rate_bitmap;

static GFont s_gothic_18, s_gothic_24;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);

  // Half width line, 2% height, specific position just off the top text
  // Y is specific to emery in this case
  graphics_fill_rect(
    ctx,
    GRect(
      0,
      scl_y_pp({.o = 60, .e = 65}),
      scl_x(500),
      scl_y(20)
    ),
    0,
    GCornerNone
  );

  // Half height line, 2% width, specific position
  graphics_fill_rect(
    ctx,
    GRect(10, 0, scl_x(20), scl_y(500)),
    0,
    GCornerNone
  );

  // 12% size and position box
  const int size = scl_x(120);
  graphics_fill_rect(ctx, GRect(size, size, size, size), 0, GCornerNone);

  // 1% sized dot, further inside only on Emery
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(
    ctx,
    GRect(
      scl_x_pp({.o = 110, .e = 130}),
      scl_x_pp({.o = 110, .e = 130}),
      scl_x(10),
      scl_x(10)
    ),
    0,
    GCornerNone
  );

  // Text in the middle third
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    "This text should appear in the middle third on any platform or screen size",
    scl_get_font(FontId_Small),
    GRect(0, scl_y(330), scl_x(1000), 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentCenter,
    NULL
  );

  // Center a GRect horizontally near the bottom, same on all platforms
  const GRect centered_h = scl_center_x(scl_grect(100, 800, 400, 50));
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, centered_h, 0, GCornerNone);

  // Center third outline box
  graphics_draw_rect(ctx, scl_center(scl_grect(10, 10, 330, 330)));

  // Text at a negative Y
  graphics_draw_text(
    ctx,
    "Off the top",
    scl_get_font(FontId_Small),
    scl_grect(0, -60, 1000, 330),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentCenter,
    NULL
  );

  // Draw image centered at the bottom - image is 24x24px as a base size, so about 15% width
  graphics_draw_bitmap_in_rect(
    ctx,
    s_rate_bitmap,
    scl_center_x(scl_grect(0, 850, 150, 150))
  );

  // Draw largest available square, inset 2px
  GRect lrg_sq = grect_inset(scl_largest_square(), GEdgeInsets(2));
  graphics_draw_rect(ctx, lrg_sq);

  APP_LOG(APP_LOG_LEVEL_INFO, "W/H: %dx%d", PS_DISP_W, PS_DISP_H);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_rate_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RATE);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  gbitmap_destroy(s_rate_bitmap);
}

static void init(void) {
  s_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_gothic_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);

  // The small font - regular screens use Gothic 18, Emery uses Gothic 24
  scl_set_fonts(FontId_Small, {.o = s_gothic_18, .e = s_gothic_24});

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
