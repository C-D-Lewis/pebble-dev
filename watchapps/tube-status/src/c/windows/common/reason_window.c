#include "reason_window.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_text_layer;

static int s_index;

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(root_layer);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(root_layer, status_bar_layer_get_layer(s_status_bar));

  char *reason = data_get_line(s_index)->reason;

  // Code from devsite to try and fit text inside a TextLayer inside a ScrollLayer
  GRect shrinking_rect = GRect(5, 0, bounds.size.w - 10, 2000);
  GSize text_size = graphics_text_layout_get_content_size(
    reason,
    scalable_get_font(SFI_Medium), 
    shrinking_rect,
    GTextOverflowModeWordWrap,
    PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft)
  );
  text_size.h += 10;
  const int x_margin = 5;
  GRect text_bounds = bounds;
  text_bounds.origin.x += x_margin;
  text_bounds.size.w -= (2 * x_margin);
  text_bounds.size.h = text_size.h;

  // Text display
  s_text_layer = text_layer_create(text_bounds);
  text_layer_set_text_alignment(
    s_text_layer,
    PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft)
  );
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(s_text_layer, scalable_get_font(SFI_Medium));
  text_layer_set_text(s_text_layer, reason);

#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_text_layer, 6);
#endif

  GEdgeInsets window_insets = (GEdgeInsets) {
    .top = STATUS_BAR_LAYER_HEIGHT - 1,
  };
  GRect scroll_rect = grect_inset(bounds, window_insets);

  s_scroll_layer = scroll_layer_create(scroll_rect);
  scroll_layer_set_content_size(s_scroll_layer, text_size);
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(root_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(s_text_layer);
  status_bar_layer_destroy(s_status_bar);

  window_destroy(window);
  s_window = NULL;
}

void reason_window_push(int index) {
  s_index = index;

  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorWhite);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
