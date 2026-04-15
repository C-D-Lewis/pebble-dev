#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  AppState *app_state = data_get_app_state();
  PersistData *persist_data = data_get_persist_data();

  // Weather
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_TEMP)) {
    app_state->current_temp = packet_get_integer(iter, MESSAGE_KEY_CURRENT_TEMP);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_CODE)) {
    app_state->current_code = packet_get_integer(iter, MESSAGE_KEY_CURRENT_CODE);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_HUMIDITY)) {
    app_state->current_humidity_perc = packet_get_integer(iter, MESSAGE_KEY_CURRENT_HUMIDITY);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_WIND)) {
    app_state->current_wind_kmh = packet_get_integer(iter, MESSAGE_KEY_CURRENT_WIND);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_SUNRISE)) {
    snprintf(app_state->sunrise, sizeof(app_state->sunrise), "%s", packet_get_string(iter, MESSAGE_KEY_SUNRISE));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_SUNSET)) {
    snprintf(app_state->sunset, sizeof(app_state->sunset), "%s", packet_get_string(iter, MESSAGE_KEY_SUNSET));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_TEMP_ARR)) {
    snprintf(app_state->temp_arr, sizeof(app_state->temp_arr), "%s", packet_get_string(iter, MESSAGE_KEY_TEMP_ARR));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_PRECIP_ARR)) {
    snprintf(
      app_state->precip_arr,
      sizeof(app_state->precip_arr),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_PRECIP_ARR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CODE_ARR)) {
    snprintf(app_state->code_arr, sizeof(app_state->code_arr), "%s", packet_get_string(iter, MESSAGE_KEY_CODE_ARR));
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
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_WIND_UNIT)) {
    snprintf(
      persist_data->wind_unit,
      sizeof(persist_data->wind_unit),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_WIND_UNIT)
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
