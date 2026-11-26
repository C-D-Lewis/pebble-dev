#include "comm.h"

static AppTimer *s_timeout_timer;

static int s_lines_received;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  if (packet_contains_key(iter, MESSAGE_KEY_LINE_TYPE)) {
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d", index);

    LineData *line_data = data_get_line(s_lines_received);
    line_data->type = packet_get_integer(iter, MESSAGE_KEY_LINE_TYPE);

    char *status = packet_get_string(iter, MESSAGE_KEY_LINE_STATUS);
    snprintf(line_data->state, strlen(status) + 1 /* EOF */, "%s", status);

    char *reason = packet_get_string(iter, MESSAGE_KEY_LINE_REASON);
    snprintf(line_data->reason, strlen(reason) + 1 /* EOF */, "%s", reason);

    s_lines_received += 1;
  }

  if (packet_contains_key(iter, MESSAGE_KEY_FLAG_LINE_COUNT)) {
    data_set_progress_max(packet_get_integer(iter, MESSAGE_KEY_FLAG_LINE_COUNT));
    splash_window_update();
  }
  data_set_progress(s_lines_received);

  if (
    packet_contains_key(iter, MESSAGE_KEY_FLAG_IS_COMPLETE) &&
    packet_get_integer(iter, MESSAGE_KEY_FLAG_IS_COMPLETE) == 1
  ) {
    line_window_push();
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped! Reason: %d", reason);
}

void comm_init() {
  app_message_open(COMM_INBOX_SIZE, COMM_OUTBOX_SIZE);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);

}

void comm_deinit() {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
}

int comm_get_lines_received() {
  return s_lines_received;
}
