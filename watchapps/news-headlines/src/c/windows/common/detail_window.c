#include "detail_window.h"

static Window *s_window;
static TextLayer *s_desc_layer;
static Layer *s_deco_layer;

static int s_index;

static void deco_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  const int margin = 3;
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(margin, 2, bounds.size.w - 6, 9), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(margin, 14, bounds.size.w - 6, 2), 0, GCornerNone);
  graphics_fill_rect(
    ctx,
    grect_inset(bounds, GEdgeInsets(bounds.size.h - 4, margin, 2, margin)),
    0,
    GCornerNone
  );
}

static void update_data() {
  Story *story = data_get_story(s_index);

  text_layer_set_text(s_desc_layer, story->description);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const int x_margin = 4;
  s_desc_layer = text_layer_create(
    GRect(x_margin, STATUS_BAR_LAYER_HEIGHT, bounds.size.w - (2 * x_margin), bounds.size.h)
  );
  text_layer_set_background_color(s_desc_layer, GColorClear);
  text_layer_set_text_alignment(
    s_desc_layer,
    PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft)
  );
  text_layer_set_overflow_mode(s_desc_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_font(s_desc_layer, scalable_get_font(SFI_Medium));
  layer_add_child(window_layer, text_layer_get_layer(s_desc_layer));
#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_desc_layer, 6);
#endif

  s_deco_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_deco_layer, deco_layer_update_proc);
  layer_add_child(window_layer, s_deco_layer);

  update_data();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_desc_layer);
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
