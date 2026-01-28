#include "detail_window.h"

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_desc_layer;
static Layer *s_deco_layer;

static int s_index;

static void deco_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  const int margin = scl_x(15);
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(
    ctx,
    GRect(margin, margin, bounds.size.w - (2 * margin), scl_y(40)),
    0,
    GCornerNone
  );
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    GRect(margin, scl_y(60), bounds.size.w - (2 * margin), scl_y(15)),
    0,
    GCornerNone
  );
  graphics_fill_rect(
    ctx,
    grect_inset(bounds, GEdgeInsets(bounds.size.h - 4, margin, 2, margin)),
    0,
    GCornerNone
  );
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  Story *story = data_get_story(s_index);

  const int y_margin = scl_y(60);
  
  // Code from devsite to try and fit text inside a TextLayer inside a ScrollLayer
  GRect shrinking_rect = GRect(2, 0, bounds.size.w - 4, 2000);
  GSize text_size = graphics_text_layout_get_content_size(
    story->description,
    scl_get_font(SFI_Medium), 
    shrinking_rect,
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft
  );
  text_size.h += 10;
  GRect text_bounds = bounds;
  text_bounds.origin.x += 2;
  text_bounds.size.w -= 4;
  text_bounds.size.h = text_size.h;

  s_desc_layer = text_layer_create(text_bounds);
  text_layer_set_font(s_desc_layer, scl_get_font(SFI_Medium));
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentLeft);
  text_layer_set_text(s_desc_layer, story->description);

#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_desc_layer, 6);
#endif

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
