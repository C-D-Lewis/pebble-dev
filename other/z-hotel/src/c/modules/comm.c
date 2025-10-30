#include "comm.h"

static AppTimer *s_timeout_timer;

void set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  int index = 0;
  if (packet_contains_key(iter, MESSAGE_KEY_HotelIndex)) {
    index = packet_get_integer(iter, MESSAGE_KEY_HotelIndex);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d", index);

    HotelData *hotel_data = data_get_hotel(index);
    hotel_data->index = index;

    char *name = packet_get_string(iter, MESSAGE_KEY_HotelName);
    snprintf(hotel_data->name, strlen(name) + 1 /* EOF */, "%s", name);

    char *summary = packet_get_string(iter, MESSAGE_KEY_HotelSummary);
    snprintf(hotel_data->summary, strlen(summary) + 1 /* EOF */, "%s", summary);
  }

  data_set_progress(index);
  if (packet_contains_key(iter, MESSAGE_KEY_FlagHotelCount)) {
    data_set_progress_max(packet_get_integer(iter, MESSAGE_KEY_FlagHotelCount));
    splash_window_update();
  }

  if (packet_contains_key(iter, MESSAGE_KEY_FlagUpdatedAt)) {
    char *updated_at = packet_get_string(iter, MESSAGE_KEY_FlagUpdatedAt);
    data_set_updated_at(updated_at);
  }

  if (
    packet_contains_key(iter, MESSAGE_KEY_FlagIsComplete) &&
    packet_get_integer(iter, MESSAGE_KEY_FlagIsComplete) == 1
  ) {
    set_fast(false);
    list_window_push();
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