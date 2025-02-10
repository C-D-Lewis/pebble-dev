#include "detail_window.h"

typedef enum {
  ScrollStatePageOne = 0,
  ScrollStatePageTwo
} ScrollState;

static Window *s_window;
static TextLayer *s_desc_layer;
static Layer *s_up_indicator_layer, *s_down_indicator_layer;
static Layer *s_img_hint_Layer;
static ScrollLayer *s_scroll_layer;

static ScrollState s_scroll_state;
static int s_index;

/*********************************** Clicks ***********************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
#if defined(PBL_COLOR)
  // Show ImageWindow
  Story *this_story = data_get_story(s_index);
  if(connection_service_peek_pebble_app_connection() && this_story->has_image) {
    image_window_push(s_index);
  }
#endif
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/*********************************** Window ***********************************/

static void hint_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  Story *this_story = data_get_story(s_index);
  if(this_story->has_image) {
    graphics_context_set_fill_color(ctx, GColorWhite);

    const int radius = 15;
    graphics_fill_circle(ctx, GPoint(bounds.size.w + (radius / 2), ((bounds.size.h - (2 * radius)) / 2) + radius), radius);
  }
}

static void update_data() {
  if(s_desc_layer) {
    Story *story = data_get_story(s_index);

    text_layer_set_text(s_desc_layer, story->description);

    // Content available?
    ContentIndicator *indicator = scroll_layer_get_content_indicator(s_scroll_layer);
    content_indicator_set_content_available(indicator, ContentIndicatorDirectionUp,
      (s_scroll_state == ScrollStatePageTwo));
    content_indicator_set_content_available(indicator, ContentIndicatorDirectionDown,
      (s_scroll_state == ScrollStatePageOne));

    // Dumb ScrollLayer
    GRect bounds = layer_get_frame(text_layer_get_layer(s_desc_layer));
    GSize text_size = text_layer_get_content_size(s_desc_layer);
    GRect frame = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, text_size.h + STATUS_BAR_LAYER_HEIGHT);
    layer_set_frame(text_layer_get_layer(s_desc_layer), frame);
    layer_set_bounds(text_layer_get_layer(s_desc_layer), frame);
    scroll_layer_set_content_size(s_scroll_layer, GSize(text_size.w, text_size.h));
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_img_hint_Layer = layer_create(bounds);
#if defined(PBL_COLOR)
  layer_set_update_proc(s_img_hint_Layer, hint_update_proc);
  layer_add_child(window_layer, s_img_hint_Layer);
#endif

  const int x_margin = 0;
  const int y_margin = 0;
  s_desc_layer = text_layer_create(GRect(x_margin, y_margin, bounds.size.w - (2 * x_margin), 2000));
  text_layer_set_text_color(s_desc_layer, GColorWhite);
  text_layer_set_background_color(s_desc_layer, GColorClear);
  text_layer_set_text_alignment(s_desc_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  text_layer_set_overflow_mode(s_desc_layer, GTextOverflowModeWordWrap);

  switch(settings_get_font_size()) {
    case FontSizeSmall:
      text_layer_set_font(s_desc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      break;
    case FontSizeLarge:
      text_layer_set_font(s_desc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
      break;
  }

  const GEdgeInsets insets = GEdgeInsets(
    PBL_IF_ROUND_ELSE(STATUS_BAR_LAYER_HEIGHT, STATUS_BAR_LAYER_HEIGHT / 2), 1, STATUS_BAR_LAYER_HEIGHT, 1);
  s_scroll_layer = scroll_layer_create(grect_inset(bounds, insets));
  scroll_layer_set_click_config_onto_window(s_scroll_layer, s_window);
  scroll_layer_set_shadow_hidden(s_scroll_layer, true);
  scroll_layer_set_paging(s_scroll_layer, true);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_desc_layer));
  scroll_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks) {
    .click_config_provider = click_config_provider
  });
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

#if defined(PBL_ROUND)  // Crash on Aplite if this is included for some reason
  text_layer_enable_screen_text_flow_and_paging(s_desc_layer, 6);
#endif

  s_up_indicator_layer = layer_create(grect_inset(bounds, GEdgeInsets(0, 0, bounds.size.h - STATUS_BAR_LAYER_HEIGHT, 0)));
  layer_add_child(window_layer, s_up_indicator_layer);
  s_down_indicator_layer = layer_create(grect_inset(bounds, GEdgeInsets(bounds.size.h - STATUS_BAR_LAYER_HEIGHT, 0, 0, 0)));
  layer_add_child(window_layer, s_down_indicator_layer);

  ContentIndicator *indicator = scroll_layer_get_content_indicator(s_scroll_layer);
  indicator = scroll_layer_get_content_indicator(s_scroll_layer);
  content_indicator_configure_direction(indicator, ContentIndicatorDirectionUp, &(ContentIndicatorConfig) {
    .layer = s_up_indicator_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorWhite,
      .background = GColorDarkCandyAppleRed
    }
  });
  content_indicator_configure_direction(indicator, ContentIndicatorDirectionDown, &(ContentIndicatorConfig) {
    .layer = s_down_indicator_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorWhite,
      .background = GColorDarkCandyAppleRed
    }
  });
}

static void window_unload(Window *window) {
  text_layer_destroy(s_desc_layer);
  scroll_layer_destroy(s_scroll_layer);
  layer_destroy(s_up_indicator_layer);
  layer_destroy(s_down_indicator_layer);
  layer_destroy(s_img_hint_Layer);

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void detail_window_push(int index) {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorDarkCandyAppleRed);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  s_scroll_state = ScrollStatePageOne;
  s_index = index;
  update_data();
}
