#include <pebble.h>

#define KEY_TOGGLE 0

static Window *s_main_window;
static TextLayer *s_output_layer;

static void send(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  dict_write_int(iter, key, &value, sizeof(int), true);
  
  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  send(KEY_TOGGLE, 0);  //Value can be any int for now
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void main_window_load(Window *window) {
  //Create TextLayer
  s_output_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_text(s_output_layer, "Press SELECT to toggle Spark pin D0");
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_output_layer));
}

static void main_window_unload(Window *window) {
  //Destroy TextLayer
  text_layer_destroy(s_output_layer);
}

static void init(void) {
  //Create Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  //Prepare AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  //Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
