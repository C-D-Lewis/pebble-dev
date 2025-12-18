#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);
  while (t) {
    uint32_t key = t->key;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received key: %d", (int)key);
    if (key == MESSAGE_KEY_BatteryAndBluetooth ||
        key == MESSAGE_KEY_WeatherStatus ||
        key == MESSAGE_KEY_SecondTick) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Received boolean: %d", (int)t->value->int32);
      data_set_boolean(key, t->value->int32 == 1);
    }

    if (key == MESSAGE_KEY_WeatherString) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Received weather: %s", t->value->cstring);
      data_set_weather_string(t->value->cstring);
    }

    t = dict_read_next(iter);
  }

  main_window_reload();
}

void comm_init() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(512, 256);
}

void comm_request_weather() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet value = TupletInteger(MESSAGE_KEY_WeatherRequest, 0);
  dict_write_tuplet(iter, &value);
  
  app_message_outbox_send();
}
