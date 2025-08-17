#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_read_first(iter);
  while(t) {
    data_set(t->key, t->value->int32);
    t = dict_read_next(iter);
  }

  main_window_reload_config();
}

void comm_init() {
  const int buffer_size = 128;
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(buffer_size, buffer_size);
}
