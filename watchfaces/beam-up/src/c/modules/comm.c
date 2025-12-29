#include "comm.h"

// TODO Use pebble-packet once flint is supported
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = dict_find(iter, DataKeyDate);
  if (t) data_set_boolean_setting(DataKeyDate, (t->value->int8 == 1));

  t = dict_find(iter, DataKeyAnimations);
  if (t) data_set_boolean_setting(DataKeyAnimations, (t->value->int8 == 1));

  t = dict_find(iter, DataKeyBTIndicator);
  if (t) data_set_boolean_setting(DataKeyBTIndicator, (t->value->int8 == 1));

  t = dict_find(iter, DataKeyHourlyVibration);
  if (t) data_set_boolean_setting(DataKeyHourlyVibration, (t->value->int8 == 1));

  t = dict_find(iter, DataKeySleep);
  if (t) data_set_boolean_setting(DataKeySleep, (t->value->int8 == 1));

#if defined(PBL_COLOR)
  t = dict_find(iter, DataKeyForegroundColor);
  if (t) data_set_foreground_color(GColorFromHEX(t->value->int32));

  t = dict_find(iter, DataKeyBackgroundColor);
  if (t) data_set_background_color(GColorFromHEX(t->value->int32));
#endif

  main_window_reload_config(true);
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() { }
