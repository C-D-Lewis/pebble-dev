#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);
  while (t) {
    uint32_t key = t->key;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received key: %d", (int)key);
    if (key == MESSAGE_KEY_BatteryMeter ||
        key == MESSAGE_KEY_BluetoothAlert ||
        key == MESSAGE_KEY_DashedLine ||
        key == MESSAGE_KEY_SecondTick) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Received boolean: %d", (int)t->value->int32);
      data_set_boolean(key, t->value->int32 == 1);
    }

    if (key == MESSAGE_KEY_BackgroundColor ||
        key == MESSAGE_KEY_DateColor ||
        key == MESSAGE_KEY_TimeColor ||
        key == MESSAGE_KEY_BracketColor ||
        key == MESSAGE_KEY_LineColor ||
        key == MESSAGE_KEY_ComplicationColor) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Received color: %d", (int)t->value->int32);
      data_set_color(key, GColorFromHEX(t->value->int32));
    }

    t = dict_read_next(iter);
  }

  // Exit to reload
  main_window_reload();
}

void comm_init() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(1024, 256);
}
