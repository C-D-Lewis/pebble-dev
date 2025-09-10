#include "reason_window.h"

typedef enum {
  ScrollStatePageOne = 0,
  ScrollStatePageTwo
} ScrollState;

static Window *s_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_reason_layer;
static Layer *s_up_indicator_layer, *s_down_indicator_layer;
static ScrollLayer *s_scroll_layer;

static ScrollState s_scroll_state;
static int s_index;

/*********************************** Window ***********************************/

static void update_data() {
  // Get reason string
  char *reason = data_get_line_reason(s_index);
#ifdef PBL_PLATFORM_EMERY
  const char* font_key = FONT_KEY_GOTHIC_24_BOLD;
#else
  const char* font_key = FONT_KEY_GOTHIC_18_BOLD;
#endif
  text_layer_set_font(s_reason_layer, fonts_get_system_font(font_key));
  text_layer_set_text(s_reason_layer, reason);

  // Content available?
  ContentIndicator *indicator = scroll_layer_get_content_indicator(s_scroll_layer);
  content_indicator_set_content_available(
    indicator,
    ContentIndicatorDirectionUp,
    (s_scroll_state == ScrollStatePageTwo)
  );
  content_indicator_set_content_available(
    indicator,
    ContentIndicatorDirectionDown,
    (s_scroll_state == ScrollStatePageOne)
  );

  // Dumb ScrollLayer
  Layer *text_layer = text_layer_get_layer(s_reason_layer);
  GRect bounds = layer_get_frame(text_layer);
  GSize text_size = text_layer_get_content_size(s_reason_layer);

  const int x_margin = 3;
  GRect frame = GRect(
    bounds.origin.x,
    bounds.origin.y,
    bounds.size.w - x_margin,
    text_size.h
  );
  layer_set_frame(text_layer, frame);
  layer_set_bounds(text_layer, frame);
  scroll_layer_set_content_size(
    s_scroll_layer,
    GSize(text_size.w, text_size.h)
  );
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  const int x_margin = 2;
  const int y_margin = 0;
  s_reason_layer = text_layer_create(
    GRect(x_margin, y_margin, bounds.size.w - (2 * x_margin), 2000)
  );
  text_layer_set_background_color(s_reason_layer, GColorClear);
  text_layer_set_text_alignment(
    s_reason_layer,
    PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft)
  );
  text_layer_set_overflow_mode(s_reason_layer, GTextOverflowModeWordWrap);

  const GEdgeInsets insets = GEdgeInsets(STATUS_BAR_LAYER_HEIGHT, 0, STATUS_BAR_LAYER_HEIGHT, 0);
  s_scroll_layer = scroll_layer_create(grect_inset(bounds, insets));
  scroll_layer_set_click_config_onto_window(s_scroll_layer, s_window);
  scroll_layer_set_shadow_hidden(s_scroll_layer, true);
  scroll_layer_set_paging(s_scroll_layer, true);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_reason_layer));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

#if defined(PBL_ROUND)
  text_layer_enable_screen_text_flow_and_paging(s_reason_layer, 6);
#endif

  s_up_indicator_layer = layer_create(
    grect_inset(
      bounds,
      GEdgeInsets(STATUS_BAR_LAYER_HEIGHT, 0, bounds.size.h - (2 * STATUS_BAR_LAYER_HEIGHT), 0)
    )
  );
  layer_add_child(window_layer, s_up_indicator_layer);
  s_down_indicator_layer = layer_create(
    grect_inset(bounds, GEdgeInsets(bounds.size.h - STATUS_BAR_LAYER_HEIGHT, 0, 0, 0))
  );
  layer_add_child(window_layer, s_down_indicator_layer);

  ContentIndicator *indicator = scroll_layer_get_content_indicator(s_scroll_layer);
  indicator = scroll_layer_get_content_indicator(s_scroll_layer);
  content_indicator_configure_direction(
    indicator,
    ContentIndicatorDirectionUp,
    &(ContentIndicatorConfig) {
      .layer = s_up_indicator_layer,
      .times_out = false,
      .alignment = GAlignCenter,
      .colors = {
        .foreground = GColorWhite,
        .background = GColorLightGray
      }
  });
  content_indicator_configure_direction(
    indicator,
    ContentIndicatorDirectionDown,
    &(ContentIndicatorConfig) {
      .layer = s_down_indicator_layer,
      .times_out = false,
      .alignment = GAlignCenter,
      .colors = {
        .foreground = GColorWhite,
        .background = GColorLightGray
      }
  });

  update_data();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_reason_layer);
  scroll_layer_destroy(s_scroll_layer);
  status_bar_layer_destroy(s_status_bar);
  layer_destroy(s_up_indicator_layer);
  layer_destroy(s_down_indicator_layer);

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void reason_window_push(int index) {
  s_scroll_state = ScrollStatePageOne;
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
