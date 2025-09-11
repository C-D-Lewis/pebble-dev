#include "reason_window.h"

static Window *s_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_reason_layer;

static int s_index;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  const int x_margin = 2;
  GEdgeInsets menu_insets = (GEdgeInsets) {
    .top = STATUS_BAR_LAYER_HEIGHT,
    .left = x_margin,
    .right = x_margin
  };
  GRect text_bounds = grect_inset(bounds, menu_insets);

  // Text display
  s_reason_layer = text_layer_create(text_bounds);
  text_layer_set_text_alignment(s_reason_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_overflow_mode(s_reason_layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(window_layer, text_layer_get_layer(s_reason_layer));

#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_reason_layer, 6);
#endif

  // Get reason string
  char *reason = data_get_line_reason(s_index);
#ifdef PBL_PLATFORM_EMERY
  const char* font_key = FONT_KEY_GOTHIC_24_BOLD;
#else
  const char* font_key = FONT_KEY_GOTHIC_18_BOLD;
#endif
  text_layer_set_font(s_reason_layer, fonts_get_system_font(font_key));
  text_layer_set_text(s_reason_layer, reason);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_reason_layer);
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
