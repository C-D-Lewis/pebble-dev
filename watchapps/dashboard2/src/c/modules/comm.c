#include "comm.h"

static time_t s_sync_start;

static void packet_failed_handler() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Packet failed");
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  AppState *app_state = data_get_app_state();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  if (packet_contains_key(iter, MESSAGE_KEY_COMPAT_PROTOCOL_VERSION)) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Compat protocol version: %d", packet_get_integer(iter, MESSAGE_KEY_COMPAT_PROTOCOL_VERSION));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Current protocol version: %d", COMPAT_PROTOCOL_VERSION);
    app_state->compat_protocol_version = packet_get_integer(
      iter,
      MESSAGE_KEY_COMPAT_PROTOCOL_VERSION
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_TOGGLE_ORDER)) {
    snprintf(
      app_state->toggle_order,
      sizeof(app_state->toggle_order),
      packet_get_string(iter, MESSAGE_KEY_TOGGLE_ORDER)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_DEVICE_NAME)) {
    snprintf(
      app_state->device_name,
      sizeof(app_state->device_name),
      packet_get_string(iter, MESSAGE_KEY_DEVICE_NAME)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_BATTERY_PERC)) {
    snprintf(
      app_state->battery_perc,
      sizeof(app_state->battery_perc),
      packet_get_string(iter, MESSAGE_KEY_BATTERY_PERC)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_FREE_SPACE)) {
    snprintf(
      app_state->free_space,
      sizeof(app_state->free_space),
      packet_get_string(iter, MESSAGE_KEY_FREE_SPACE)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_FREE_SPACE_PERC)) {
    snprintf(
      app_state->free_space_perc,
      sizeof(app_state->free_space_perc),
      packet_get_string(iter, MESSAGE_KEY_FREE_SPACE_PERC)
    );
  }

  // Min. delay for UX
  time_t now = time(NULL);
  if (now - s_sync_start < 1) {
    app_timer_register(500, main_window_update, NULL);
  } else {
    main_window_update();
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped: %d", reason);
}

void comm_init() {
  app_message_open(COMM_INBOX_SIZE, COMM_OUTBOX_SIZE);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);

  packet_init();

  s_sync_start = time(NULL);
}

void comm_deinit() {

}

void comm_sync_data() {
#ifdef USE_TEST_DATA
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Using test data");
  app_timer_register(1500, data_test_data_handler, NULL);
  return;
#endif

  // Sync current settings and states from Android
  if (packet_begin()) {
    packet_put_boolean(MESSAGE_KEY_SYNC_REQUEST, true);
    packet_send(packet_failed_handler);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "packet_begin() failed");
  }
}
