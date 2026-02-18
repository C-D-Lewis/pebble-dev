#include "comm.h"

// Can't use pebble-packet due to low memory on aplite. Sad packet noises.

#ifdef FEATURE_SYNC
static int s_sync_index = STATUS_EMPTY;
#endif

static void add_push_pin_data(DictionaryIterator *iter) {
#if defined(TEST_TIMELINE_PIN)
  const int days = 1;
  const int rate = 4;
#else
  const int days = data_calculate_days_remaining();
  const int rate = data_calculate_avg_discharge_rate();
#endif

  if (!util_is_not_status(days) || !util_is_not_status(rate)) return;

  dict_write_int32(iter, MESSAGE_KEY_PUSH_PIN, 1);
  dict_write_int32(iter, MESSAGE_KEY_DAYS_REMAINING, days);
  dict_write_int32(iter, MESSAGE_KEY_DISCHARGE_RATE, rate);
}

#ifdef FEATURE_SYNC
static void send_int(uint32_t key) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int32(iter, key, 1);
  app_message_outbox_send();
}

static void add_get_sync_info_data(DictionaryIterator *iter) {
  dict_write_int32(iter, MESSAGE_KEY_GET_SYNC_INFO, 1);
}

static void send_sample(int index) {
  const Sample *s = data_get_sample(index);
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int32(iter, MESSAGE_KEY_SYNC_SAMPLE, 1);
  dict_write_int32(iter, MESSAGE_KEY_SAMPLE_TIMESTAMP, s->timestamp);
  dict_write_int32(iter, MESSAGE_KEY_SAMPLE_RESULT, s->result);
  dict_write_int32(iter, MESSAGE_KEY_SAMPLE_CHARGE_PERC, s->charge_perc);
  dict_write_int32(iter, MESSAGE_KEY_SAMPLE_TIME_DIFF, s->time_diff);
  dict_write_int32(iter, MESSAGE_KEY_SAMPLE_CHARGE_DIFF, s->charge_diff);
  dict_write_int32(iter, MESSAGE_KEY_SAMPLE_RATE, s->rate);
  app_message_outbox_send();
}

// Need timer so inbox handler completes for JS
static void send_timer_handler(void *context) {
  const int index = (int)context;
  // APP_LOG(APP_LOG_LEVEL_INFO, "Send %d", index);
  s_sync_index = index;
  send_sample(index);
}

static void send_next_sample_after(int last_ts) {
  // Locate index of next sample that has a newer timestamp
  int index = data_get_log_length() - 1;
  while (index >= 0) {
    Sample *s = data_get_sample(index);
    if (s->timestamp > last_ts) break; // Handles STATUS_EMPTY too

    index--;
  }

  if (index < 0) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "syncd");
    return;
  }

  // Break appmessage event loop
  app_timer_register(100, send_timer_handler, (void*)index);
}
#endif

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Out f %d", (int)reason);
}

void out_sent_handler(DictionaryIterator *iterator, void *context) {
#ifdef FEATURE_SYNC
  if (s_sync_index < 0) return;

  const Sample *s = data_get_sample(s_sync_index);
  send_next_sample_after(s->timestamp);
#endif
}

void inbox_received_handler(DictionaryIterator *iter, void *context) {
  PersistData *persist_data = data_get_persist_data();

  // Things to do when JS is ready
  // We can only respond with one AppMessage at a time it seems
  Tuple *t = dict_find(iter, MESSAGE_KEY_READY);
  if (t) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (persist_data->push_timeline_pins) add_push_pin_data(iter);
#if defined(FEATURE_SYNC)
    add_get_sync_info_data(iter);
#endif
    app_message_outbox_send();
    return;
  }

#ifdef FEATURE_SYNC
  // Response to sync info request
  t = dict_find(iter, MESSAGE_KEY_SYNC_TIMESTAMP);
  if (t) {
    const int last_ts = (int)t->value->int32;

    t = dict_find(iter, MESSAGE_KEY_SYNC_COUNT);
    const int sync_count = (int)t->value->int32;
    // APP_LOG(APP_LOG_LEVEL_INFO, "Sync: %d (%d)", last_ts, sync_count);

    AppState *app_state = data_get_app_state();
    app_state->sync_count = sync_count;

    t = dict_find(iter, MESSAGE_KEY_STAT_TOTAL_DAYS);
    app_state->stat_total_days = t->value->int32;

    t = dict_find(iter, MESSAGE_KEY_STAT_ALL_TIME_RATE);
    app_state->stat_all_time_rate = t->value->int32;

    t = dict_find(iter, MESSAGE_KEY_STAT_LAST_WEEK_RATE);
    app_state->stat_last_week_rate = t->value->int32;

    t = dict_find(iter, MESSAGE_KEY_STAT_NUM_CHARGES);
    app_state->stat_num_charges = t->value->int32;

    t = dict_find(iter, MESSAGE_KEY_STAT_MTBC);
    app_state->stat_mtbc = t->value->int32;

    // TODO: Non-looping way to update the UI when sync progresses
    settings_window_reload();
    stats_window_reload();

    // TODO: If watch is empty and phone has data, option to sync back to watch?
    //       Only if it is intended that JS data persists when watchapp is uninstalled
    //       This might be stale data though.

#if !defined(USE_TEST_DATA) || (defined(USE_TEST_DATA) && defined(SYNC_TEST_DATA))
    // Continue sync after summary is received, but not every time
    send_next_sample_after(last_ts);
#endif
    return;
  }
#endif
}

void comm_init() {
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_inbox_received(inbox_received_handler);

  // Consider Aplite when adding to these
#ifdef FEATURE_SYNC
  const int inbox_size = 128;
  const int outbox_size = inbox_size;
#else
  const int inbox_size = 32;
  const int outbox_size = 64;
#endif
  app_message_open(inbox_size, outbox_size);
}

void comm_deinit() {}

#ifdef FEATURE_SYNC
void comm_request_deletion() {
  send_int(MESSAGE_KEY_SYNC_DELETE);
}

void comm_request_sync_stats() {
  send_int(MESSAGE_KEY_GET_STATS);
}
#endif
