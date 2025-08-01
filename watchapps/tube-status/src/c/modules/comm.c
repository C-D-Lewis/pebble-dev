#include "comm.h"

static AppTimer *s_timeout_timer;

// static void get_data_handler(void *context) {
//   comm_request_data();
// }

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d", packet_get_size(iter));

  // For some reaosn this prevents future messages from being received???
  // if (packet_contains_key(iter, MESSAGE_KEY_JSReady)) {
  //   // JS is ready, acknowledge
  //   app_timer_register(500, get_data_handler, NULL);
  //   return;
  // }

  // Extract all lines
  // if (packet_contains_key(iter, MESSAGE_KEY_LineTypeBakerloo)) {
  //   // LineTypeBakerloo
  //   char *line_state = data_get_line_state(LineTypeBakerloo);
  //   snprintf(line_state, 64 /* Max length */, "%s", packet_get_string(iter, MESSAGE_KEY_LineTypeBakerloo));
  //   if (VERBOSE) {
  //     APP_LOG(APP_LOG_LEVEL_DEBUG, "Got line %d: %s", LineTypeBakerloo, line_state);
  //   }
  //   advance = true;
  // }

  Tuple *t = dict_read_first(iter);
  while (t) {
    // With "messageKeys", it appears to start from 10000
    int key = t->key - 10000;
    if (VERBOSE) APP_LOG(APP_LOG_LEVEL_DEBUG, "Received key: %d, value: %s", key, t->value->cstring);

    // Line keys are linear from 0
    if (key >= 0 && key < LineTypeMax) {
      char *line_state = data_get_line_state(key);
      snprintf(line_state, strlen(t->value->cstring) + 1 /* EOF */, "%s", t->value->cstring);
      // if (VERBOSE) APP_LOG(APP_LOG_LEVEL_DEBUG, "Got line %d: %s", key, t->value->cstring);
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

void comm_request_data() {
  // Tasty packets!
  if (packet_begin()) {
    packet_put_integer(MESSAGE_KEY_JSReady, 0);
    packet_send(failed_callback);
  }
}
