#include "splash_window.h"

#define BAR_WIDTH 75
#define SPLASH_LOGO_RADIUS 20

static Window *s_window;
static Layer *s_logo_layer, *s_bar_layer;

static void logo_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, SPLASH_LOGO_RADIUS - 1);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, (5 * (SPLASH_LOGO_RADIUS - 1)) / 7); 
}

/**
 * Simple progress bar with one pixel inset
 */
static void progress_bar_update_proc(Layer *layer, GContext *ctx) {
  const int margin = 4;

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(margin, margin), margin);
  graphics_fill_rect(ctx, GRect(margin, 0, BAR_WIDTH - (2 * margin), (2 * margin) + 1), 0, GCornerNone);
  graphics_fill_circle(ctx, GPoint(BAR_WIDTH - margin, margin), margin);

  int width = (data_get_progress() * BAR_WIDTH) / LineTypeMax;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(margin, margin), margin - 1);
  graphics_fill_rect(ctx, GRect(margin + 1, 1, width - 2, (2 * margin) - 1), 0, GCornerNone);
  graphics_fill_circle(ctx, GPoint(margin + width, margin), margin - 1);
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

  const int logo_y_margin = (bounds.size.h - logo_rect.size.h) / 2;
  const int x_margin = (bounds.size.w - BAR_WIDTH) / 2;
  s_bar_layer = layer_create(
    GRect(
      x_margin,
      logo_y_margin + logo_rect.size.h + 17,
      bounds.size.w - (2 * x_margin),
      10
    )
  );
  layer_set_update_proc(s_bar_layer, progress_bar_update_proc);
  layer_add_child(window_layer, s_bar_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_logo_layer);
  layer_destroy(s_bar_layer);

  window_destroy(window);
  s_window = NULL;

  window_stack_pop_all(true);
}

void splash_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorWhite);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}

void splash_window_update() {
  layer_mark_dirty(s_bar_layer);
}
