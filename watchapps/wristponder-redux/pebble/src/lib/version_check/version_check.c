#include "version_check.h"

static VersionCheckCallback *s_callback_ptr;

static void send(char *version) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, KEY_VERSION_CHECK_VERSION, version);
  app_message_outbox_send();
}

static void in_received_handler(DictionaryIterator *iter, void *context) { 
  Tuple *t = dict_read_first(iter);
  while(t) {
    int key = t->key;

    switch(key) {
    //Match!
    case KEY_VERSION_CHECK_SUCCESS:
      app_log(APP_LOG_LEVEL_ERROR, "version_check", 0, "Version check passed!");
      s_callback_ptr(true);
      break;

    //Fail!
    case KEY_VERSION_CHECK_FAILURE:
      app_log(APP_LOG_LEVEL_ERROR, "version_check", 0, "Version check failed!");
      s_callback_ptr(false);
      break;

    //Else
    default:
      app_log(APP_LOG_LEVEL_ERROR, "version_check", 0, "Response key not KEY_VERSION_CHECK_SUCCESS or KEY_VERSION_CHECK_FAILURE, was %d", key);
      s_callback_ptr(false);
      break;
    }

    //Finally
    t = dict_read_next(iter);
  }
}

void version_check(char *version, VersionCheckCallback *callback) {
  //Register our callback
  app_message_register_inbox_received(in_received_handler);

  //Store pointer to callback
  s_callback_ptr = callback;

  //Send check request
  send(version);
}
