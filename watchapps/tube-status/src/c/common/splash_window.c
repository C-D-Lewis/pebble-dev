#include "splash_window.h"

#define BAR_WIDTH            75
#define SPLASH_LOGO_RADIUS   20

static Window *s_window;
static Layer *s_logo_layer;

static int s_first_view = true;

static void logo_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, SPLASH_LOGO_RADIUS - 1);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, (5 * (SPLASH_LOGO_RADIUS - 1)) / 7); 
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  int diameter = 2 * SPLASH_LOGO_RADIUS;
  GEdgeInsets logo_insets = (GEdgeInsets) {
    .top = (bounds.size.h - diameter) / 2,
    .right = (bounds.size.w - diameter) / 2,
    .bottom = (bounds.size.h - diameter) / 2,
    .left = (bounds.size.w - diameter) / 2
  };
  GRect logo_rect = grect_inset(bounds, logo_insets);

  s_logo_layer = layer_create(logo_rect);
  layer_set_update_proc(s_logo_layer, logo_update_proc);
  layer_add_child(window_layer, s_logo_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_logo_layer);

  window_destroy(window);
  s_window = NULL;

  window_stack_pop_all(true);
}

static void window_appear(Window *window) {
  if(s_first_view) {
    // JS probably not ready
    s_first_view = false;
  } else {
    // We are revisiting from settings
    comm_update_subscription_state();
  }
}

void splash_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorWhite);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear
    });
  }
  window_stack_push(s_window, true);
}
