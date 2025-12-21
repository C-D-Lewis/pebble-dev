#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Config message received");

  // TODO: Use pebble-packet here
  Tuple *t = dict_read_first(iter);
  while (t) {
    uint32_t key = t->key;
    if (
      key == MESSAGE_KEY_EnableDate || key == MESSAGE_KEY_EnableDay || 
      key == MESSAGE_KEY_EnableBT || key == MESSAGE_KEY_EnableBattery || 
      key == MESSAGE_KEY_EnableSecondHand || key == MESSAGE_KEY_EnableSteps
    ) {
      data_set_enable(key, t->value->int32 != 0);
    } else if (
      key == MESSAGE_KEY_ColorBackground || key == MESSAGE_KEY_ColorHourMinutes || 
      key == MESSAGE_KEY_ColorSeconds || key == MESSAGE_KEY_ColorNotches || 
      key == MESSAGE_KEY_ColorMonthDay || key == MESSAGE_KEY_ColorDate
    ) {
      data_set_color(key, (int)t->value->int32);
    } else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown config key received: %d", (int)key);
    }

    t = dict_read_next(iter);
  }

  main_window_reload_config();
}

void comm_init() {
  const int buffer_size = 256;
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(buffer_size, buffer_size);
}
