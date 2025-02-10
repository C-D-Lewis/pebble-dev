#include "comm.h"

static AppTimer *s_timeout_timer;

static void get_data_handler(void *context) {
  comm_request_data();
}

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  bool advance = false;

  Tuple *t = dict_read_first(iter);
  while(t) {
    int key = t->key;

    // Line keys are linear from 0
    if(key >=0 && key < LineTypeMax) {
      char *line_state = data_get_line_state(key);
      snprintf(line_state, strlen(t->value->cstring) + 1 /* EOF */, "%s", t->value->cstring);

      if(VERBOSE) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Got line %d: %s", key, t->value->cstring);
      }

      advance = true;
    } else {
      // Link keys
      switch(key) {
        case AppMessageKeyJSReady: 
          // JS is ready, acknowledge
          app_timer_register(500, get_data_handler, NULL);
          return; // Only time we don't advance the Window
        case AppMessageKeyServerStatus:
          // The server is up! (A response, timeout is handled here)
          settings_window_update_server_status(ServerStatusUp);

          if(s_timeout_timer) {
            app_timer_cancel(s_timeout_timer);
            s_timeout_timer = NULL;
          }
          break;
        default:
          APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key: %d", key);
          break;
      }
    }

    t = dict_read_next(iter);
  }

  if(advance) {
    // All are here
    line_window_push();
  }
}

void comm_init() {
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(COMM_INBOX_SIZE, COMM_OUTBOX_SIZE);
}

void comm_deinit() {
  if(s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
}

static void timeout_handler(void *context) {
  s_timeout_timer = NULL;
  settings_window_update_server_status(ServerStatusTimeout);
}

void comm_get_server_status() {
  if(s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
  }
  s_timeout_timer = app_timer_register(10000, timeout_handler, NULL);

  if(packet_begin()) {
    packet_put_integer(AppMessageKeyServerStatus, 0);
    packet_send();
  }
}

void comm_request_data() {
  // Tasty packets
  if(packet_begin()) {
    packet_put_integer(AppMessageKeyJSReady, 0);
    packet_send();
  }
}

void comm_update_subscription_state() {
  if(packet_begin()) {
    packet_put_integer(AppMessageKeySubscriptionState, (int)settings_get_subscription_state());
    packet_send();
  }
}
