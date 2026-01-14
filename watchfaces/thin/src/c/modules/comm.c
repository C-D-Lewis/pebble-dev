#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Config message received");

  // Clay sends all keys at once
  data_set_enable(MESSAGE_KEY_EnableDate, packet_get_boolean(iter, MESSAGE_KEY_EnableDate));
  data_set_enable(MESSAGE_KEY_EnableBT, packet_get_boolean(iter, MESSAGE_KEY_EnableBT));
  data_set_enable(MESSAGE_KEY_EnableBattery, packet_get_boolean(iter, MESSAGE_KEY_EnableBattery));
  data_set_enable(
    MESSAGE_KEY_EnableSecondHand,
    packet_get_boolean(iter, MESSAGE_KEY_EnableSecondHand)
  );
  data_set_enable(MESSAGE_KEY_EnableSteps, packet_get_boolean(iter, MESSAGE_KEY_EnableSteps));
  data_set_color(
    MESSAGE_KEY_ColorBackground,
    GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorBackground))
  );
  data_set_color(
    MESSAGE_KEY_ColorHourMinutes,
    GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorHourMinutes))
  );
  data_set_color(
    MESSAGE_KEY_ColorSeconds,
    GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorSeconds))
  );
  data_set_color(
    MESSAGE_KEY_ColorNotches,
    GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorNotches))
  );
  data_set_color(
    MESSAGE_KEY_ColorMonthDay,
    GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorMonthDay))
  );
  data_set_color(
    MESSAGE_KEY_ColorDate,
    GColorFromHEX(packet_get_integer(iter, MESSAGE_KEY_ColorDate))
  );

  main_window_reload_config();
}

void comm_init() {
  packet_init();

  app_message_register_inbox_received(in_recv_handler);
  app_message_open(256, 256);
}
