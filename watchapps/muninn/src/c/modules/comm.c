#include "comm.h"

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send outbox");
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Check for days remaining and rate
  Tuple *pin_set_t = dict_find(iter, MESSAGE_KEY_PIN_SET);
  if (pin_set_t && pin_set_t->value->int32 == 1) {
    data_set_pin_set_time(time(NULL));
    APP_LOG(APP_LOG_LEVEL_INFO, "Pin set time updated");
  }
}

void comm_init() {
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(512, 512);
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
    APP_LOG(APP_LOG_LEVEL_INFO, "Not enough data for pin prediction");
    return;
  }

  // TODO: Once pebble-events and pebble-packet are updated for flint
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet days_tuple = TupletInteger(MESSAGE_KEY_DAYS_REMAINING, days);
  dict_write_tuplet(iter, &days_tuple);
  Tuplet rate_tuple = TupletInteger(MESSAGE_KEY_DISCHARGE_RATE, rate);
  dict_write_tuplet(iter, &rate_tuple);
  
  app_message_outbox_send();
}
