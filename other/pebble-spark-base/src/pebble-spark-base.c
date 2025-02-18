/**
 * Pebble-Spark Base
 * @Chris_DL
 * 
 * Use this base project to send Pebble button events to the Spark Core 
 * running spark_src/main.ino
 *
 * Enter your Core device ID and account access token in src/js/pebble-js-app.js
 *
 * Button mappings:
 * UP - up();
 * SELECT - select();
 * DOWN - down();
 * Long-press SELECT - longselect();
 */

#include <pebble.h>

// Configuration
#define LONG_CLICK_MS 500
#define SHOW_LOGS true

// AppMessage keys
#define KEY_SELECT      0
#define KEY_DOWN        1
#define KEY_LONG_SELECT 2
#define KEY_UP          3
#define KEY_PING        4

// UI
static Window *s_main_window;
static TextLayer *s_output_layer, *s_status_layer;

/********************************* AppMessage *********************************/

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read first Tuple
  Tuple *t = dict_read_first(iter);

  // Read all Tuples
  while(t != NULL) {
    int key = t->key;

    switch(key) {
      //Spark reply
      case KEY_PING:
        if(strcmp(t->value->cstring, "1") == 0) {
          text_layer_set_text(s_status_layer, "Status: OK");
        } else {
          text_layer_set_text(s_status_layer, "Status: Spark API error");
        }
      break;

      // Unknown key
      default:
        if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key received: %d", key);
      break;
    }

    // Read any next Tuple
    t = dict_read_next(iter);
  }
}

static void send(int key, int value) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, key, &value, sizeof(int), true);

  app_message_outbox_send();
}

/*********************************** Clicks ***********************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_status_layer, "Status: Sending SELECT...");
  send(KEY_SELECT, 0);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {  
  text_layer_set_text(s_status_layer, "Status: Sending L-SELECT...");
  send(KEY_LONG_SELECT, 0);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_status_layer, "Status: Sending UP...");
  send(KEY_UP, 0);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_status_layer, "Status: Sending DOWN...");
  send(KEY_DOWN, 0);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, LONG_CLICK_MS, select_long_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/********************************** UI Setup **********************************/

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Output TextLayer
  s_output_layer = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_text(s_output_layer, "Press a button to send event to Spark");
  text_layer_set_text_alignment(s_output_layer, GTextAlignmentCenter);
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));

  s_status_layer = text_layer_create(GRect(5, 130, bounds.size.w, 30));
  text_layer_set_text(s_status_layer, "Status: Ready");
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentLeft);
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));
}

static void main_window_unload(Window *window) {
  // Destroy everything
  text_layer_destroy(s_output_layer);
  text_layer_destroy(s_status_layer);
}

/********************************** App ***************************************/

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Open AppMessage
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
