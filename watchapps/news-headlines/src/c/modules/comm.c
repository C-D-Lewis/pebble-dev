#include "comm.h"

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = NULL;

  // JS Ready
  t = dict_find(iter, AppKeyReady);
  if (t) {
    splash_window_begin();
  }

  // Quantity?
  t = dict_find(iter, AppKeyQuantity);
  if (t) {
    int quantity = t->value->int32;
    data_set_quantity(quantity);

    splash_window_set_progress(0);

    if (SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Got quantity: %d", data_get_quantity());
  }

  //  Story
  t = dict_find(iter, AppKeyIndex);
  if (t) {
    int index = t->value->int32;
    data_store_story(iter, index);

    splash_window_set_progress(index);
  }
}

void comm_init() {
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(1024, 512);
  comm_set_fast(true);
}

void comm_send_settings() {
  DictionaryIterator *out;
  app_message_outbox_begin(&out);

  int value = settings_get_category();
  dict_write_int(out, AppKeySettingsCategory, &value, sizeof(int), true);
  value = settings_get_number_of_stories();
  dict_write_int(out, AppKeySettingsNumStories, &value, sizeof(int), true);
  value = settings_get_region();
  dict_write_int(out, AppKeySettingsRegion, &value, sizeof(int), true);

  app_message_outbox_send();
}

void comm_set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}
