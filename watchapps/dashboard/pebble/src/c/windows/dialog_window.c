// TODO: Make pebble package
#include "dialog_window.h"

static Window *s_window;
static TextLayer *s_text_layer;
static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_tick_bitmap;

#define MAX_LENGTH 128
static char s_text_buffer[MAX_LENGTH];

static void update_layout() {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  const int margin = 5;
  const int y_margin = PBL_IF_ROUND_ELSE(5 * margin, margin);
  GRect text_bounds = grect_inset(bounds, GEdgeInsets(y_margin, ACTION_BAR_WIDTH + margin, margin, margin));
  layer_set_frame(text_layer_get_layer(s_text_layer), text_bounds);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) { 
  window_stack_pop(true);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(bounds);
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_alignment(s_text_layer, PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentLeft));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  s_tick_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK);

  s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_tick_bitmap);
  action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);
  action_bar_layer_add_to_window(s_action_bar_layer, window);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  action_bar_layer_destroy(s_action_bar_layer);
  gbitmap_destroy(s_tick_bitmap);

  window_destroy(s_window);
  s_window = NULL;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Dialog window exited with heap free %d", (int)heap_bytes_free());
}

/************************************ API *************************************/

// Text should be per-platform newlined as appropriate
void dialog_window_push(char *text) {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);

  window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
  text_layer_set_text_color(s_text_layer, GColorBlack);

  snprintf(s_text_buffer, sizeof(s_text_buffer), "%s", text);
  text_layer_set_text(s_text_layer, s_text_buffer);

  update_layout();
}
