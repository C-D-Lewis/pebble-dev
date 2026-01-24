#include "comm.h"

// Can't use pebble-packet due to low memory on aplite. Sad packet noises.

static int s_send_index = -1;

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Out fail");
}

void out_sent_handler(DictionaryIterator *iterator, void *context) {
  // Send next now if still sending
  if (s_send_index > 0) {
    // Search again from here
    const Sample *s = data_get_sample(s_send_index);
    comm_send_samples(s->timestamp);
  }
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Things to do when JS is ready
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_READY);
  if (ready_tuple) {
    if (data_get_push_timeline_pins()) {
      comm_push_timeline_pins();
    }

    comm_get_last_timestamp();
    return;
  }

  // Response to last timestamp request
  Tuple *timestamp_tuple = dict_find(iter, MESSAGE_KEY_LAST_TIMESTAMP);
  if (timestamp_tuple) {
    int last_ts = (int)timestamp_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "last ts %d", last_ts);

    // Send all samples newer than this (greater timestamp)
    comm_send_samples(last_ts);
    return;
  }
}

void comm_init() {
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
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

  if (!util_is_not_status(days) || !util_is_not_status(rate)) return;

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet days_tuple = TupletInteger(MESSAGE_KEY_DAYS_REMAINING, days);
  dict_write_tuplet(iter, &days_tuple);
  Tuplet rate_tuple = TupletInteger(MESSAGE_KEY_DISCHARGE_RATE, rate);
  dict_write_tuplet(iter, &rate_tuple);
  app_message_outbox_send();
}

void comm_get_last_timestamp() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet days_tuple = TupletInteger(MESSAGE_KEY_GET_LAST_TIMESTAMP, 1);
  dict_write_tuplet(iter, &days_tuple);
  app_message_outbox_send();
}

static void send_sample(int index) {
  const Sample *s = data_get_sample(index);
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet timestamp_tuple = TupletInteger(MESSAGE_KEY_EXPORT_TIMESTAMP, s->timestamp);
  dict_write_tuplet(iter, &timestamp_tuple);
  Tuplet result_tuple = TupletInteger(MESSAGE_KEY_EXPORT_RESULT, s->result);
  dict_write_tuplet(iter, &result_tuple);
  Tuplet charge_perc_tuple = TupletInteger(MESSAGE_KEY_EXPORT_CHARGE_PERC, s->charge_perc);
  dict_write_tuplet(iter, &charge_perc_tuple);
  Tuplet time_diff_tuple = TupletInteger(MESSAGE_KEY_EXPORT_TIME_DIFF, s->time_diff);
  dict_write_tuplet(iter, &time_diff_tuple);
  Tuplet charge_diff_tuple = TupletInteger(MESSAGE_KEY_EXPORT_CHARGE_DIFF, s->charge_diff);
  dict_write_tuplet(iter, &charge_diff_tuple);
  Tuplet rate_tuple = TupletInteger(MESSAGE_KEY_EXPORT_RATE, s->rate);
  dict_write_tuplet(iter, &rate_tuple);
  app_message_outbox_send();
}

// Need timer so inbox handler completes for JS
static void send_timer_handler(void *context) {
  int index = (int)context;

  // Send the one we found
  APP_LOG(APP_LOG_LEVEL_INFO, "Send %d", index);
  s_send_index = index;
  send_sample(index);
}

void comm_send_samples(int last_ts) {
  // Locate index of next sample that has a newer timestamp
  int index = data_get_log_length() - 1;
  while (index >= 0) {
    Sample *s = data_get_sample(index);
    if (s->timestamp > last_ts) break;

    index--;
  }

  // Reached end
  if (index < 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "end export");
    return;
  }

  app_timer_register(100, send_timer_handler, (void*)index);
}
