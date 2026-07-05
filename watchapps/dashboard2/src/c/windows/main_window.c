#include "main_window.h"

#define HEADER_HEIGHT scl_y(330)

static Window *s_window;
static Layer *s_header_layer, *s_toggles_layer;

static void header_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
}

static void toggles_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();
  
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    app_state->sync_toggle_order,
    scl_get_font(SFI_Medium),
    layer_get_bounds(layer),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  // Initial positions
  GRect header_bounds = GRect(
    0,
    -(HEADER_HEIGHT - STATUS_BAR_LAYER_HEIGHT),
    bounds.size.w,
    HEADER_HEIGHT
  );
  GRect toggles_bounds = grect_inset(bounds, GEdgeInsets(STATUS_BAR_LAYER_HEIGHT, 0, 0, 0));
  s_header_layer = layer_create(header_bounds);
  layer_set_update_proc(s_header_layer, header_update_proc);
  layer_add_child(window_layer, s_header_layer);

  s_toggles_layer = layer_create(toggles_bounds);
  layer_set_update_proc(s_toggles_layer, toggles_update_proc);
  layer_add_child(window_layer, s_toggles_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_header_layer);
  layer_destroy(s_toggles_layer);
  
  window_destroy(window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);

  comm_sync_data();
}

void main_window_update() {
  layer_mark_dirty(s_header_layer);
  layer_mark_dirty(s_toggles_layer);
}
