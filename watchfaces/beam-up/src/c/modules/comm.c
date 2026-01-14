#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Clay sends all keys at once
  data_set_boolean_setting(DataKeyDate, packet_get_boolean(iter, DataKeyDate));
  data_set_boolean_setting(DataKeyAnimations, packet_get_boolean(iter, DataKeyAnimations));
  data_set_boolean_setting(DataKeyBTIndicator, packet_get_boolean(iter, DataKeyBTIndicator));
  data_set_boolean_setting(DataKeyHourlyVibration, packet_get_boolean(iter, DataKeyHourlyVibration));
  data_set_boolean_setting(DataKeySleep, packet_get_boolean(iter, DataKeySleep));

#if defined(PBL_COLOR)
  data_set_foreground_color(GColorFromHEX(packet_get_integer(iter, DataKeyForegroundColor)));
  data_set_background_color(GColorFromHEX(packet_get_integer(iter, DataKeyBackgroundColor)));
#endif

  main_window_reload_config(true);
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() {}
