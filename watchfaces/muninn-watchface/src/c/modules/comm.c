#include "comm.h"

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  AppState *app_state = data_get_app_state();

  // The only message is battery data
  if (packet_contains_key(iter, MESSAGE_KEY_BATTERY_DAYS_REMAINING)) {
    app_state->days_remaining = packet_get_integer(iter, MESSAGE_KEY_BATTERY_DAYS_REMAINING);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_BATTERY_LIFE)) {
    app_state->battery_life = packet_get_integer(iter, MESSAGE_KEY_BATTERY_LIFE);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_BATTERY_LAST_WEEK_RATE)) {
    app_state->last_week_rate = packet_get_integer(iter, MESSAGE_KEY_BATTERY_LAST_WEEK_RATE);
  }

  main_window_reload();
}

void comm_init() {
  app_message_open(128, 128);
  app_message_register_inbox_received(inbox_received_handler);

  packet_init();
}

void comm_deinit() {}
