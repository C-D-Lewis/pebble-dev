#include "comm.h"

static AppTimer *s_timeout_timer;

void set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  // Handle incoming line configuration data
  if (packet_contains_key(iter, MESSAGE_KEY_ConfigLineIndex)) {
    int index = packet_get_integer(iter, MESSAGE_KEY_ConfigLineIndex);
    char *name = packet_get_string(iter, MESSAGE_KEY_ConfigLineName);
    uint32_t color = (uint32_t)packet_get_integer(iter, MESSAGE_KEY_ConfigLineColor);
    bool striped = packet_get_integer(iter, MESSAGE_KEY_ConfigLineStriped) != 0;

    data_set_line_config(index, name, color, striped);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Configured line %d: %s", index, name);
    data_set_progress(data_get_progress() + 1);
    splash_window_update();
    return;
  }

  // Handle incoming line status data
  if (packet_contains_key(iter, MESSAGE_KEY_LineStatusIndex)) {
    int index = packet_get_integer(iter, MESSAGE_KEY_LineStatusIndex);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d", index);

    LineData *line_data = data_get_line(index);
    line_data->index = index;

    char *status = packet_get_string(iter, MESSAGE_KEY_LineStatus);
    snprintf(line_data->state, strlen(status) + 1 /* EOF */, "%s", status);

    char *reason = packet_get_string(iter, MESSAGE_KEY_LineReason);
    snprintf(line_data->reason, strlen(reason) + 1 /* EOF */, "%s", reason);

    if (packet_contains_key(iter, MESSAGE_KEY_LineStatusSeverity)) {
      line_data->severity = (StatusSeverity)packet_get_integer(iter, MESSAGE_KEY_LineStatusSeverity);
    } else {
      line_data->severity = StatusSeverityGood;
    }
  }

  // Update progress for any status or config message
  data_set_progress(data_get_progress() + 1);
  if (packet_contains_key(iter, MESSAGE_KEY_FlagLineCount)) {
    data_set_progress_max(packet_get_integer(iter, MESSAGE_KEY_FlagLineCount) * 2);
    splash_window_update();
  }

  if (
    packet_contains_key(iter, MESSAGE_KEY_FlagIsComplete) &&
    packet_get_integer(iter, MESSAGE_KEY_FlagIsComplete) == 1
  ) {
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