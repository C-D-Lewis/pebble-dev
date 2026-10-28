#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  PersistData *persist_data = data_get_persist_data();
  AppState *app_state = data_get_app_state();

  // Weather
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_TEMP)) {
    app_state->current_temp = packet_get_integer(iter, MESSAGE_KEY_CURRENT_TEMP);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_CODE)) {
    app_state->current_code = packet_get_integer(iter, MESSAGE_KEY_CURRENT_CODE);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_SUNRISE)) {
    snprintf(app_state->sunrise, sizeof(app_state->sunrise), "%s", packet_get_string(iter, MESSAGE_KEY_SUNRISE));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_SUNSET)) {
    snprintf(app_state->sunset, sizeof(app_state->sunset), "%s", packet_get_string(iter, MESSAGE_KEY_SUNSET));
  }

  // Config
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_TEMP_UNIT)) {
    snprintf(
      persist_data->temp_unit,
      sizeof(persist_data->temp_unit),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_TEMP_UNIT)
    );
  }

  // Other
  if (packet_contains_key(iter, MESSAGE_KEY_WEATHER_ERROR)) {
    app_state->current_code = WEATHER_ERROR;
  }

  main_window_reload();
}

void comm_request_weather() {
  if (packet_begin()) {
    packet_put_integer(MESSAGE_KEY_REQUEST_WEATHER, 1);
    packet_send(NULL);
  }
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() {}
