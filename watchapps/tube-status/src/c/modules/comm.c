#include "comm.h"

static AppTimer *s_timeout_timer;

void set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}

void request_default_transit_system() {
  comm_request_transit_system(-1);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  if (packet_contains_key(iter, MESSAGE_KEY_Type)) {
    char *type = packet_get_string(iter, MESSAGE_KEY_Type);

    if (strcmp(type, "ready") == 0) {
#if defined(APP_VARIANT_TUBE_STATUS)
      request_default_transit_system();
#else
      DictionaryIterator *out_iter;
      AppMessageResult result = app_message_outbox_begin(&out_iter);
      if (result == APP_MSG_OK) {
        dict_write_int8(out_iter, MESSAGE_KEY_RequestAvailableTransitSystems, 1);
        app_message_outbox_send();
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Requested available transit systems");
      } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to begin outbox: %d", result);
      }
      return;
#endif
    } else if (strcmp(type, "lineConfig") == 0) {
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
    } else if (strcmp(type, "lineStatus") == 0) {
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

      if (
        packet_contains_key(iter, MESSAGE_KEY_FlagIsComplete) &&
        packet_get_integer(iter, MESSAGE_KEY_FlagIsComplete) == 1
      ) {
        set_fast(false);
        line_window_push();
      }
    } else if (strcmp(type, "availableTransitSystem") == 0) {
      if (packet_contains_key(iter, MESSAGE_KEY_AvailableTransitSystemName) &&
        packet_contains_key(iter, MESSAGE_KEY_AvailableTransitSystemRegion) &&
        packet_contains_key(iter, MESSAGE_KEY_AvailableTransitSystemIndex)) {
        char *name = packet_get_string(iter, MESSAGE_KEY_AvailableTransitSystemName);
        char *region = packet_get_string(iter, MESSAGE_KEY_AvailableTransitSystemRegion);
        int index = packet_get_integer(iter, MESSAGE_KEY_AvailableTransitSystemIndex);

        data_set_transit_system_data(index, name, region);

        if (
          packet_contains_key(iter, MESSAGE_KEY_FlagIsComplete) &&
          packet_get_integer(iter, MESSAGE_KEY_FlagIsComplete) == 1
        ) {
          transit_system_selection_window_push();
        } 
      } else {
        request_default_transit_system();
      }
    } else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown message type: %s", type);
      return;
    }
  }

  // Update progress for any messaging
  data_set_progress(data_get_progress() + 1);
  if (packet_contains_key(iter, MESSAGE_KEY_FlagLineCount)) {
    data_set_progress_max(packet_get_integer(iter, MESSAGE_KEY_FlagLineCount) * 2);
    splash_window_update();
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

void comm_request_transit_system(int index) {
  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK) {
    dict_write_int8(out_iter, MESSAGE_KEY_RequestTransitSystem, index);
    app_message_outbox_send();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requested transit system data for index %d", index);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to begin outbox: %d", result);
  }
  return;
}