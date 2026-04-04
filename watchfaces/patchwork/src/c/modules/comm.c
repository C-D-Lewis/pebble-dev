#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  data_set_palette(packet_get_string(iter, MESSAGE_KEY_PALETTE));

  main_window_reload();
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() {}
