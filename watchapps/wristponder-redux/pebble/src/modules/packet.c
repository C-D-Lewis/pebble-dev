#include "packet.h"

static DictionaryIterator *s_outbox;

bool packet_begin() {
  AppMessageResult result = app_message_outbox_begin(&s_outbox);
  if(result == APP_MSG_OK) {
    return true;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error beginning packet: %d", (int)result);
    return false;
  }
}

void packet_set_type(int type) {
  packet_add_int(AppKeyPacketType, type);
}

void packet_add_int(int key, int value) {
  dict_write_int(s_outbox, key, &value, sizeof(int), true);
}

void packet_add_string(int key, char *string) {
  dict_write_cstring(s_outbox, key, string);
}

bool packet_send() {
  AppMessageResult result = app_message_outbox_send();
  if(result == APP_MSG_OK) {
    return true;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending packet: %d", (int)result);
    return false;
  }
}
