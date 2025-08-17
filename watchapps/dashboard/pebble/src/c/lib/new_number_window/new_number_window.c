#include "new_number_window.h"

static Window *s_window;
static TextLayer *s_title_layer, *s_value_layer;
static ActionBarLayer *s_action_bar;
static StatusBarLayer *s_status_bar;

static GBitmap *s_up_bitmap, *s_select_bitmap, *s_down_bitmap;
static NewNumberWindowCallback *s_callback;
static char s_title_buffer[32];
static int s_min, s_max, s_step, s_value;

static void set_value() {
  static char s_buff[8];
  snprintf(s_buff, sizeof(s_buff), "%d", s_value);
  text_layer_set_text(s_value_layer, s_buff);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Accept value
  window_stack_pop(true);
  s_callback(s_value);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(s_value < s_max) {
    s_value += s_step;
  }
  set_value();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(s_value > s_min) {
    s_value -= s_step;
  }
  set_value();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 200, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 200, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UP);
  s_select_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SELECT);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOWN);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP, s_up_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_select_bitmap);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_down_bitmap);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  action_bar_layer_add_to_window(s_action_bar, window);

  const int x_margin = PBL_IF_ROUND_ELSE(0, ACTION_BAR_WIDTH);
  const int y_margin = PBL_IF_ROUND_ELSE(35, 25);
  s_title_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(y_margin, x_margin, 0, 0)));
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_title_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  const int text_size = 50;
  const int y_centered = (bounds.size.h - text_size) / 2;
  s_value_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(y_centered, x_margin, 0, 0)));
  text_layer_set_font(s_value_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_background_color(s_value_layer, GColorClear);
  text_layer_set_text_alignment(s_value_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(s_value_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_value_layer));

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_bar, GColorBlack, GColorWhite);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
}

static void window_appear(Window *window) {
  // Reset state
  s_value = s_min;
  text_layer_set_text(s_title_layer, s_title_buffer);
  set_value();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_value_layer);
  action_bar_layer_destroy(s_action_bar);
  status_bar_layer_destroy(s_status_bar);
  
  gbitmap_destroy(s_up_bitmap);
  gbitmap_destroy(s_select_bitmap);
  gbitmap_destroy(s_down_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

void new_number_window_push(char *title, int min, int max, int step, NewNumberWindowCallback callback) {
  s_callback = callback;
  s_min = min;
  s_max = max;
  s_step = step;
  snprintf(s_title_buffer, sizeof(s_title_buffer), "%s", title);

  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear
    });
  }
  window_stack_push(s_window, true);

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap free: %d", (int)heap_bytes_free()); // Pushing the limits of Aplite
}
