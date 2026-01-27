#include "comm.h"

// Can't use pebble-packet due to low memory on aplite. Sad packet noises.

static int s_sync_index = -1;

static void add_push_pin_data(DictionaryIterator *iter) {
#if defined(TEST_TIMELINE_PIN)
  const int days = 1;
  const int rate = 4;
#else
  const int days = data_calculate_days_remaining();
  const int rate = data_calculate_avg_discharge_rate();
#endif

  if (!util_is_not_status(days) || !util_is_not_status(rate)) return;

  Tuplet push_tuple = TupletInteger(MESSAGE_KEY_PUSH_PIN, 1);
  dict_write_tuplet(iter, &push_tuple);
  Tuplet days_tuple = TupletInteger(MESSAGE_KEY_DAYS_REMAINING, days);
  dict_write_tuplet(iter, &days_tuple);
  Tuplet rate_tuple = TupletInteger(MESSAGE_KEY_DISCHARGE_RATE, rate);
  dict_write_tuplet(iter, &rate_tuple);
}

static void add_get_sync_info_data(DictionaryIterator *iter) {
  Tuplet sync_tuple = TupletInteger(MESSAGE_KEY_GET_SYNC_INFO, 1);
  dict_write_tuplet(iter, &sync_tuple);
}

static void send_sample(int index) {
  const Sample *s = data_get_sample(index);
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet sync_tuple = TupletInteger(MESSAGE_KEY_SYNC_SAMPLE, 1);
  dict_write_tuplet(iter, &sync_tuple);
  Tuplet timestamp_tuple = TupletInteger(MESSAGE_KEY_SAMPLE_TIMESTAMP, s->timestamp);
  dict_write_tuplet(iter, &timestamp_tuple);
  Tuplet result_tuple = TupletInteger(MESSAGE_KEY_SAMPLE_RESULT, s->result);
  dict_write_tuplet(iter, &result_tuple);
  Tuplet charge_perc_tuple = TupletInteger(MESSAGE_KEY_SAMPLE_CHARGE_PERC, s->charge_perc);
  dict_write_tuplet(iter, &charge_perc_tuple);
  Tuplet time_diff_tuple = TupletInteger(MESSAGE_KEY_SAMPLE_TIME_DIFF, s->time_diff);
  dict_write_tuplet(iter, &time_diff_tuple);
  Tuplet charge_diff_tuple = TupletInteger(MESSAGE_KEY_SAMPLE_CHARGE_DIFF, s->charge_diff);
  dict_write_tuplet(iter, &charge_diff_tuple);
  Tuplet rate_tuple = TupletInteger(MESSAGE_KEY_SAMPLE_RATE, s->rate);
  dict_write_tuplet(iter, &rate_tuple);
  app_message_outbox_send();
}

// Need timer so inbox handler completes for JS
static void send_timer_handler(void *context) {
  int index = (int)context;

  // Send the one we found
  // APP_LOG(APP_LOG_LEVEL_INFO, "Export %d", index);
  s_sync_index = index;
  send_sample(index);
}

static void send_samples_after(int last_ts) {
  // Locate index of next sample that has a newer timestamp
  int index = data_get_log_length() - 1;
  while (index >= 0) {
    Sample *s = data_get_sample(index);
    if (s->timestamp > last_ts) break; // Handles STATUS_EMPTY too

    index--;
  }

  // Reached end
  if (index < 0) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "Exported");
    return;
  }

  app_timer_register(100, send_timer_handler, (void*)index);
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Out fail");
}

void out_sent_handler(DictionaryIterator *iterator, void *context) {
  if (s_sync_index > 0) {
    // Find next item from here
    const Sample *s = data_get_sample(s_sync_index);
    send_samples_after(s->timestamp);
  }
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Things to do when JS is ready
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_READY);
  if (ready_tuple) {
    // We can only respond with one AppMessage at a time it seems
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (data_get_push_timeline_pins()) add_push_pin_data(iter);

    // Feature is WIP
    // add_get_sync_info_data(iter);
    
    app_message_outbox_send();
    return;
  }

  // Response to last timestamp request
  Tuple *timestamp_tuple = dict_find(iter, MESSAGE_KEY_SYNC_TIMESTAMP);
  if (timestamp_tuple) {
    const int last_ts = (int)timestamp_tuple->value->int32;

    Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_SYNC_COUNT);
    const int sync_count = (int)count_tuple->value->int32;
    // APP_LOG(APP_LOG_LEVEL_INFO, "Sync: %d (%d)", last_ts, sync_count);

    // TODO: If watch is empty and phone has data, option to sync back to watch?
    //       Only if it is intended that JS data persists when watchapp is uninstalled

    data_set_sync_count(sync_count);
    settings_window_reload();

    send_samples_after(last_ts);
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

void comm_request_deletion() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet sync_tuple = TupletInteger(MESSAGE_KEY_SYNC_DELETE, 1);
  dict_write_tuplet(iter, &sync_tuple);
  app_message_outbox_send();
}
