#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // TODO: Use pebble-packet?

  Tuple *t = dict_read_first(iter);
  while (t) {
    uint32_t key = t->key;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received key: %d", (int)key);
    if (key == MESSAGE_KEY_ShowDate ||
        key == MESSAGE_KEY_PlayAnimations) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Received boolean: %d", (int)t->value->int32);
      data_set_boolean(key, t->value->int32 == 1);
    }

    t = dict_read_next(iter);
  }

  // Exit to reload
  main_window_reload_config(true);
}

void comm_init() {
  events_app_message_request_inbox_size(512);
  events_app_message_request_outbox_size(512);
  events_app_message_open();

  events_app_message_register_inbox_received(inbox_received_handler, NULL);
}
