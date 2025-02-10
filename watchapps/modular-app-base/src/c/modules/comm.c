#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {

}

void comm_init(uint32_t inbox, uint32_t outbox) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}
