#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  PersistData *persist_data = data_get_persist_data();
  AppState *app_state = data_get_app_state();

  if (packet_contains_key(iter, MESSAGE_KEY_BatteryAndBluetooth)) {
    // Clay sends all keys at once
    persist_data->second_tick = packet_get_boolean(iter, MESSAGE_KEY_SecondTick);
    persist_data->battery_and_bluetooth = packet_get_boolean(iter, MESSAGE_KEY_BatteryAndBluetooth);
    snprintf(
      persist_data->complication_type,
      sizeof(persist_data->complication_type),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_ComplicationType)
    );
    persist_data->color_background = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_ColorBackground)
    );
    persist_data->color_brackets = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_ColorBrackets)
    );
    persist_data->color_datetime = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_ColorDateTime)
    );
    persist_data->color_complications = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_ColorComplications)
    );
  }

  if (packet_contains_key(iter, MESSAGE_KEY_WeatherString)) {
    snprintf(
      app_state->weather_status,
      sizeof(app_state->weather_status),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_WeatherString)
    );
  }

  main_window_reload();
}

void comm_init() {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(512, 128);
}

void comm_request_weather() {
  if (packet_begin()) {
    packet_put_integer(MESSAGE_KEY_WeatherRequest, 1);
    packet_send(NULL);
  }
}
