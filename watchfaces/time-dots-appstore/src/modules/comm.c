#include "comm.h"

static void inbox_received_handler(DictionaryIterator *inbox, void *context) {
  // Every packet will be color config!
  data_set_color(ColorTypeBackground, GColorFromHEX(packet_get_integer(inbox, MESSAGE_KEY_ColorTypeBackground)));
  data_set_color(ColorTypeRing, GColorFromHEX(packet_get_integer(inbox, MESSAGE_KEY_ColorTypeRing)));
  data_set_color(ColorTypeDotInactive, GColorFromHEX(packet_get_integer(inbox, MESSAGE_KEY_ColorTypeDotInactive)));
  data_set_color(ColorTypeDotActive, GColorFromHEX(packet_get_integer(inbox, MESSAGE_KEY_ColorTypeDotActive)));
  data_set_feature(FeatureKeyShowMinutes, packet_get_boolean(inbox, MESSAGE_KEY_ShowMinutes));
  data_set_feature(FeatureKeyBTAlert, packet_get_boolean(inbox, MESSAGE_KEY_BTAlert));

  main_window_reload_config();
  vibes_short_pulse();
}

void comm_init() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(128, 128);
}

void comm_deinit() { }
