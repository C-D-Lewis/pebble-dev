#include "pebble-packet.h"

#include <pebble-events/pebble-events.h>

#define TAG        "pebble-packet"
#define TIMEOUT_MS 5000

static PacketFailedCallback *s_failed_callback;

static DictionaryIterator *s_outbox;
static EventHandle *s_sent_handle, *s_failed_handle;
static AppTimer *s_timeout_timer;

/********************************** Internal **********************************/

static char *result_to_string(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK:               return "Success";
    case APP_MSG_SEND_TIMEOUT:     return "Send timeout";
    case APP_MSG_SEND_REJECTED:    return "Send rejected";
    case APP_MSG_NOT_CONNECTED:    return "Not connected";
    case APP_MSG_APP_NOT_RUNNING:  return "App not running";
    case APP_MSG_INVALID_ARGS:     return "Invalid arguments";
    case APP_MSG_BUSY:             return "Busy";
    case APP_MSG_BUFFER_OVERFLOW:  return "Buffer overflow";
    case APP_MSG_ALREADY_RELEASED: return "Already released";
    case APP_MSG_OUT_OF_MEMORY:    return "Out of memory";
    case APP_MSG_CLOSED:           return "Closed";
    case APP_MSG_INTERNAL_ERROR:   return "Internal error";
    case APP_MSG_INVALID_STATE:    return "Invalid state. Is AppMessage open?";
    default: {
      static char s_buff[32];
      snprintf(s_buff, sizeof(s_buff), "Unknown error (%d)", result);
      return &s_buff[0];
    }
  }
}

static void timeout_handler(void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Timed out!", TAG);
  s_timeout_timer = NULL;
  if(s_failed_callback) {
    s_failed_callback();
  }
}

static void start_timeout_timer() {
  if(s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
  s_timeout_timer = app_timer_register(TIMEOUT_MS, timeout_handler, NULL);
}

// Success!
static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
  events_app_message_unsubscribe(s_sent_handle);
  events_app_message_unsubscribe(s_failed_handle);

  if(s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
}
static void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  // Failed to send
  APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Outbox send failed! Reason: %s", TAG, result_to_string(reason));
  if(s_failed_callback) {
    s_failed_callback();
  }
}

/************************************ API *************************************/

bool packet_begin() {
  AppMessageResult r = app_message_outbox_begin(&s_outbox);
  if(r != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Error opening outbox! Reason: %s", TAG, result_to_string(r));
    return false;
  }
  return true;
}

bool packet_send(PacketFailedCallback *cb) {
  if(cb) {
    s_failed_callback = cb;
  }
  
  AppMessageResult r = app_message_outbox_send();
  if(r != APP_MSG_OK) {
    // Failed immediately
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Error sending outbox! Reason: %s", TAG, result_to_string(r));
    if(s_failed_callback) {
      s_failed_callback();
    }
    return false;
  }

  s_sent_handle = events_app_message_register_outbox_sent(outbox_sent_handler, NULL);
  s_failed_handle = events_app_message_register_outbox_failed(outbox_failed_handler, NULL);
  start_timeout_timer();
  return true;
}

bool packet_put_integer(int key, int value) {
  DictionaryResult r = dict_write_int32(s_outbox, key, value);
  if(r != DICT_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Error adding integer to outbox!", TAG);
    return false;
  }
  return true;
}

bool packet_put_string(int key, char *string) {
  DictionaryResult r = dict_write_cstring(s_outbox, key, string);
  if(r != DICT_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "%s: Error adding string to outbox!", TAG);
    return false;
  }
  return true;
}

bool packet_put_boolean(int key, bool b) {
  return packet_put_integer(key, b ? 1 : 0);
}

int packet_get_size(DictionaryIterator *inbox_iter) {
  return (int)inbox_iter->end - (int)inbox_iter->dictionary;
}

bool packet_contains_key(DictionaryIterator *inbox_iter, int key) {
  return dict_find(inbox_iter, key) != NULL;
}

int packet_get_integer(DictionaryIterator *inbox_iter, int key) {
  if(!packet_contains_key(inbox_iter, key)) {
    return 0;
  }
  return dict_find(inbox_iter, key)->value->int32;
}

char* packet_get_string(DictionaryIterator *inbox_iter, int key) {
  if(!packet_contains_key(inbox_iter, key)) {
    return NULL;
  }
  return dict_find(inbox_iter, key)->value->cstring;
}

bool packet_get_boolean(DictionaryIterator *inbox_iter, int key) {
  return packet_get_integer(inbox_iter, key) == 1;
}
