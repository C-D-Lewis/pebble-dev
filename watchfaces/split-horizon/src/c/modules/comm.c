#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Clay sends all keys at once
  data_set_boolean(MESSAGE_KEY_ShowDate, packet_get_boolean(iter, MESSAGE_KEY_ShowDate));
  data_set_boolean(MESSAGE_KEY_PlayAnimations, packet_get_boolean(iter, MESSAGE_KEY_PlayAnimations));

  main_window_reload_config(true);
}

void comm_init() {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 256);
}
