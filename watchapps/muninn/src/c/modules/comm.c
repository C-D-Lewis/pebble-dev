#include "comm.h"

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Out fail");
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Unused
}

void comm_init() {
#if !defined(PBL_PLATFORM_APLITE)
  packet_init();
#endif

  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_inbox_received(inbox_received_handler);
  // Consider Aplite when adding to these
  app_message_open(32, 128);
}

void comm_deinit() {}

void comm_push_timeline_pins() {
#if defined(TEST_TIMELINE_PIN)
  const int days = 1;
  const int rate = 4;
#else
  const int days = data_calculate_days_remaining();
  const int rate = data_calculate_avg_discharge_rate();
#endif

  if (!util_is_not_status(days) || !util_is_not_status(rate)) {
  // APP_LOG(APP_LOG_LEVEL_INFO, "Not enough data for pin prediction");
    return;
  }

  // This seems silly but here we are
#if !defined(PBL_PLATFORM_APLITE)
  if (packet_begin()) {
    packet_put_integer(MESSAGE_KEY_PUSH_PIN, 1);
    packet_put_integer(MESSAGE_KEY_DAYS_REMAINING, days);
    packet_put_integer(MESSAGE_KEY_DISCHARGE_RATE, rate);
    packet_send(NULL);
  }
#else
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet days_tuple = TupletInteger(MESSAGE_KEY_DAYS_REMAINING, days);
  dict_write_tuplet(iter, &days_tuple);
  Tuplet rate_tuple = TupletInteger(MESSAGE_KEY_DISCHARGE_RATE, rate);
  dict_write_tuplet(iter, &rate_tuple);
  
  app_message_outbox_send();
#endif
}

void data_push_sync_data(int index) {
  // Send this sample - timestamp is the unique ordered key
}
