#include "comm.h"

static AppTimer *s_timeout_timer;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  // TODO: Use pebble-packet
  Tuple *t = dict_read_first(iter);
  while (t) {
    // With "messageKeys", it appears to start from 10000
    int key = t->key - 10000;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Received key: %d, value: %s", key, t->value->cstring);

    // Line keys are linear from 0
    if (key >= 0 && key < LineTypeMax) {
      char *line_state = data_get_line_state(key);
      snprintf(line_state, strlen(t->value->cstring) + 1 /* EOF */, "%s", t->value->cstring);
    }

    t = dict_read_next(iter);
  }

  // All lines are received in the same message
  line_window_push();
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped! Reason: %d", reason);
}

void comm_init() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_open(COMM_INBOX_SIZE, COMM_OUTBOX_SIZE);
}

void comm_deinit() {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
}

static void failed_callback() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Packet send failed.");
}
