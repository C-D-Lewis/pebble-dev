#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  if (packet_contains_key(iter, MESSAGE_KEY_BatteryAndBluetooth)) {
    // Clay sends all keys at once
    data_set_boolean(
      MESSAGE_KEY_BatteryAndBluetooth,
      packet_get_boolean(iter, MESSAGE_KEY_BatteryAndBluetooth)
    );
    data_set_boolean(
      MESSAGE_KEY_WeatherStatus,
      packet_get_boolean(iter, MESSAGE_KEY_WeatherStatus)
    );
    data_set_boolean(
      MESSAGE_KEY_SecondTick,
      packet_get_boolean(iter, MESSAGE_KEY_SecondTick)
    );
    data_set_color(
      MESSAGE_KEY_ColorBackground,
      GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorBackground))
    );
    data_set_color(
      MESSAGE_KEY_ColorBrackets,
      GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorBrackets))
    );
    data_set_color(
      MESSAGE_KEY_ColorDateTime,
      GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorDateTime))
    );
    data_set_color(
      MESSAGE_KEY_ColorComplications,
      GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorComplications))
    );
  }

  if (packet_contains_key(iter, MESSAGE_KEY_WeatherString)) {
    data_set_weather_string(packet_get_string(iter, MESSAGE_KEY_WeatherString));
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
