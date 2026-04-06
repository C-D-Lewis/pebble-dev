#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_TEMP)) {
    data_set_current_temp(packet_get_integer(iter, MESSAGE_KEY_CURRENT_TEMP));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_CODE)) {
    data_set_current_code(packet_get_integer(iter, MESSAGE_KEY_CURRENT_CODE));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_TEMP_ARR)) {
    data_set_temp_arr(packet_get_string(iter, MESSAGE_KEY_TEMP_ARR));
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CODE_ARR)) {
    data_set_code_arr(packet_get_string(iter, MESSAGE_KEY_CODE_ARR));
  }

  main_window_reload();
}

void comm_request_weather() {
  if (packet_begin()) {
    packet_put_integer(MESSAGE_KEY_REQUEST_WEATHER, 1);
    packet_send(NULL);
  }
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() {}
