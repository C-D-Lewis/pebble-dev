#include "comm.h"

static AppTimer *s_timeout_timer;

void set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  int index = 0;
  if (packet_contains_key(iter, MESSAGE_KEY_LineIndex)) {
    index = packet_get_integer(iter, MESSAGE_KEY_LineIndex);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d", index);

    char *line_state = data_get_line_state(index);
    char *status = packet_get_string(iter, MESSAGE_KEY_LineStatus);
    snprintf(line_state, strlen(status) + 1 /* EOF */, "%s", status);

    char *line_reason = data_get_line_reason(index);
    char *reason = packet_get_string(iter, MESSAGE_KEY_LineReason);
    snprintf(line_reason, strlen(reason) + 1 /* EOF */, "%s", reason);
  }

  data_set_progress(index);
  splash_window_update();

  if (index == LineTypeMax - 1) {
    set_fast(false);
    line_window_push();
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped! Reason: %d", reason);
}

void comm_init() {
  events_app_message_request_inbox_size(COMM_INBOX_SIZE);
  events_app_message_request_outbox_size(COMM_OUTBOX_SIZE);
  events_app_message_open();
  events_app_message_register_inbox_received(inbox_received_handler, NULL);
  events_app_message_register_inbox_dropped(inbox_dropped_handler, NULL);

  set_fast(true);
}

void comm_deinit() {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
}