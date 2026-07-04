#include "comm.h"

void set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}

static void packet_failed_handler() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Packet failed");
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  AppState *app_state = data_get_app_state();
  snprintf(app_state->test_message, 32, packet_get_string(iter, MESSAGE_KEY_TEST_MESSAGE));
  main_window_update();

  // Respond
  if (packet_begin()) {
    packet_put_string(MESSAGE_KEY_TEST_RESPONSE, "Hello from Pebble!");
    packet_send(packet_failed_handler);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Responded");
  }

  set_fast(false);
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped: %d", reason);
}

void comm_init() {
  app_message_open(COMM_INBOX_SIZE, COMM_OUTBOX_SIZE);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);

  packet_init();

  // set_fast(true);
}

void comm_deinit() {

}