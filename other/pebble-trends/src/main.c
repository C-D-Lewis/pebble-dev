#include <pebble.h>
#include "card.h"

// AppMessage Keys
#define KEY_TITLE      0
#define KEY_TIMESTAMP  1
#define KEY_SPARK_SHOW 2
#define KEY_FINISHED   3

// Configuration
#define SHOW_LOGS false
#define MAX_TRENDS 10

// UI Elements
static Window *s_main_window;
static Card *card_array[MAX_TRENDS];

// App State
static char 
  *titles_array[MAX_TRENDS],
  *timestamp_array[MAX_TRENDS];
static int 
  send_index = 0,
  current_card = 0;

/*********************************** I/O **************************************/

// Send a AppMessage
static void send(int key, int value) {
  DictionaryIterator *iterator;
  
  app_message_outbox_begin(&iterator);
  dict_write_int(iterator, key, &value, sizeof(int), true);
  
  app_message_outbox_send();
}

// Get the number actually received
static int get_trend_quantity() {
  int result = 0;

  // Only send by JS if they do not contain non-ascii chars
  for(int i = 0; i < MAX_TRENDS; i++) {
    if(titles_array[i]) {
      result++;
    }
  }

  if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "There are %d trends.", result);

  return result;
}

// When an AppMessage is received
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);

  int last_key = 0;

  while (t != NULL) {
    last_key = t->key;

    switch(t->key) {
    case KEY_TITLE:
      titles_array[send_index] = malloc((strlen(t->value->cstring) * sizeof(char)) + 1);  // For null char
      snprintf(titles_array[send_index], (strlen(t->value->cstring) * sizeof(char)) + 1, "%s", t->value->cstring);
      break;
    case KEY_TIMESTAMP:
      timestamp_array[send_index] = malloc((strlen(t->value->cstring) * sizeof(char)) + 1);  // For null char
      snprintf(timestamp_array[send_index], (strlen(t->value->cstring) * sizeof(char)) + 1, "%s", t->value->cstring);
      break;
    case KEY_FINISHED:
      // Show first card
      current_card = 0;
      card_show(card_array[current_card]);

      // Print all cards received
      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Card readout: ======================");

      for(int i = 0; i < get_trend_quantity(); i++) {
        if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Card %d: %s", i, titles_array[i]);
      }

      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Readout ends: ======================");
      break;
    }

    t = dict_read_next(iter);
  }

  if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "send_index=%d", send_index);

  // Add a card only if a card's info was received
  if(last_key != KEY_FINISHED) {
    // Add it and hide it
    card_add_to_window(card_array[send_index], s_main_window);  // Window ptr is hacky

    card_set_title(card_array[send_index], titles_array[send_index]);
    card_set_description(card_array[send_index], timestamp_array[send_index]);

    card_hide(card_array[send_index]);

    // Alternative to inability to use arbitrary value JS keys
    send_index++; 
  }
}

/***************************** main_window ************************************/

static void main_window_load(Window *window) {
  window_set_background_color(s_main_window, GColorBlack);

  // Create Cards
  for(int i = 0; i < MAX_TRENDS; i++) {
    card_array[i] = card_create();
    // card_add_to_window(card_array[i], window);
  }

  // Experiment with one Card
  card_add_to_window(card_array[0], window);
}

static void main_window_unload(Window *window) {
  for(int i = 0; i < MAX_TRENDS; i++) {
    card_destroy(card_array[i]);
  }
}

/******************************** Clicks **************************************/

static void transition_handler(void *context) {
  Card *next = (Card*)context;

  // Show the next card
  card_show(next);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Show previous card
  if(current_card > 0) {
    // Hide last
    card_hide(card_array[current_card]);

    current_card--;

    // Show next
    app_timer_register(300, transition_handler, card_array[current_card]);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Show on Spark Core
  send(KEY_SPARK_SHOW, current_card);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Show next card
  if(current_card < get_trend_quantity() - 1) { // Prevent overscroll
    // Hide last
    card_hide(card_array[current_card]);

    current_card++;

    // Show next
    app_timer_register(300, transition_handler, card_array[current_card]);
  }
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/********************************** App ***************************************/

static void timer_callback(void *context) {
  // Start download
  send(KEY_TITLE, 0);
}

static void init() {
  // Prepare AppMessage
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Create main Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  // Start download after a short init delay TODO - start from JS?
  app_timer_register(1500, timer_callback, NULL);
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);

  // Free malloc'd data
  for(int i = 0; i < MAX_TRENDS; i++) {
    if(titles_array[i]) {
      free(titles_array[i]);
    }

    if(timestamp_array[i]) {
      free(timestamp_array[i]);
    }
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}