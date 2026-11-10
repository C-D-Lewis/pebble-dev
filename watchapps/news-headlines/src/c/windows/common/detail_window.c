#include "detail_window.h"

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_desc_layer;
static Layer *s_deco_layer;

static int s_index;

static void deco_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  const int x_margin = scl_x(15);
  const int y_margin = scl_y_pp({.o = 15, .g = 50});

  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(
    ctx,
    GRect(x_margin, y_margin, bounds.size.w - (2 * x_margin), scl_y(40)),
    0,
    GCornerNone
  );
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    GRect(
      x_margin,
      y_margin + scl_y_pp({.o = 50, .g = 50}),
      bounds.size.w - (2 * x_margin),
      scl_y(15)
    ),
    0,
    GCornerNone
  );
  graphics_fill_rect(
    ctx,
    GRect(0, bounds.size.h - (2 * y_margin), PS_DISP_W, x_margin),
    0,
    GCornerNone
  );
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  Story *story = data_get_story(s_index);

  const int x_margin = scl_x_pp({.o = 5, .g = 120});
  const int y_margin = scl_y_pp({.o = 60, .g = 140});
  
  // Code from devsite to try and fit text inside a TextLayer inside a ScrollLayer
  GRect shrinking_rect = GRect(x_margin, 0, bounds.size.w - (2 * x_margin), 2000);
  GTextAlignment alignment = PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft);
  GSize text_size = graphics_text_layout_get_content_size(
    story->description,
    scl_get_font(SFI_Medium), 
    shrinking_rect,
    GTextOverflowModeWordWrap,
    alignment
  );
  text_size.h += 10;
  GRect text_bounds = bounds;
  text_bounds.origin.x += x_margin;
  text_bounds.size.w -= 2 * x_margin;
  text_bounds.size.h = text_size.h;

  s_desc_layer = text_layer_create(text_bounds);
  text_layer_set_font(s_desc_layer, scl_get_font(SFI_Medium));
  text_layer_set_text_alignment(s_desc_layer, alignment);
  text_layer_set_text(s_desc_layer, story->description);

  s_scroll_layer = scroll_layer_create(
    GRect(0, y_margin, bounds.size.w, bounds.size.h - y_margin - scl_y(25))
  );
  scroll_layer_set_content_size(s_scroll_layer, text_size);
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_desc_layer));
  layer_add_child(root_layer, scroll_layer_get_layer(s_scroll_layer));

  s_deco_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_deco_layer, deco_layer_update_proc);
  layer_add_child(root_layer, s_deco_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_desc_layer);
  scroll_layer_destroy(s_scroll_layer);
  layer_destroy(s_deco_layer);

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void detail_window_push(int index) {
  s_index = index;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
