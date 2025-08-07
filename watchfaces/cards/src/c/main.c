#include <pebble.h>

#include "card.h"
#include "include/alert.h"

#define MAX_BUFFER_LENGTH 32
#define KEY_LOCATION 0
#define KEY_CONDITIONS 1
#define KEY_TEMPERATURE 2

static Window* main_window;
static Card *card;

static bool in_progress = false;
static char *last_location_buff, *last_conditions_buff;

static void next_card();

/**
 * Allow retying
 */
static void retry_callback(void *context) {
  next_card();
}

/**
 * Upon spawn completion
 */
static void spawn_callback(void *context) {
  in_progress = false;
}

/**
 * Upon despawn completion
 */
static void despawn_callback(void *context) {
  // Spawn next
  card_destroy(card);
  card = card_add(main_window, last_location_buff, last_conditions_buff);
  card_spawn(card);

  // Finally
  app_timer_register(600, (AppTimerCallback)spawn_callback, NULL);
}

/**
 * Show next card
 */
static void next_card() {
  if (!in_progress) {
    card_despawn(card);

    in_progress = true;
    app_timer_register(600, (AppTimerCallback)despawn_callback, NULL);
  } else {
    // Schedule later
    app_timer_register(15000, (AppTimerCallback)retry_callback, NULL);
  }
}

static void main_window_load(Window *window) {
  window_set_background_color(window, GColorBlack);

  card = card_add(window, last_location_buff, last_conditions_buff);
}

static void main_window_unload(Window *window) {
  card_destroy(card);

  window_destroy(window);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  next_card();

  if(tick_time->tm_min % 15 == 0) {
    // Request new weather
    cl_send_int(0, 0);
  }
}

static void bt_handler(bool connected) {
  vibes_long_pulse();

  next_card();
}

static void process_tuple(Tuple *t) {
  // Which type?
  switch(t->key) {
    case KEY_LOCATION:
      snprintf(last_location_buff, MAX_BUFFER_LENGTH * sizeof(char), "%s", t->value->cstring);

      APP_LOG(APP_LOG_LEVEL_DEBUG, "RECV: %s", last_location_buff);
      break;
    case KEY_CONDITIONS:
      snprintf(last_conditions_buff, MAX_BUFFER_LENGTH * sizeof(char), "%s", t->value->cstring);

      APP_LOG(APP_LOG_LEVEL_DEBUG, "RECV: %s", last_conditions_buff);
      break;
    }
}

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);

  while (t != NULL) {
    process_tuple(t);

    t = dict_read_next(iter);
  }

  next_card();
}

static void init() {
  last_conditions_buff = malloc(MAX_BUFFER_LENGTH * sizeof(char));
  last_location_buff = malloc(MAX_BUFFER_LENGTH * sizeof(char));

  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler)tick_handler);
  bluetooth_connection_service_subscribe((BluetoothConnectionHandler)bt_handler);
  battery_state_service_subscribe((BatteryStateHandler)next_card);

  cl_app_message_open(1024, 128, in_recv_handler);

  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(main_window, true);
}

static void deinit() {
  free(last_location_buff);
  free(last_conditions_buff);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
