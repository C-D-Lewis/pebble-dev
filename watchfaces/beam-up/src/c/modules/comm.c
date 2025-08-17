#include "comm.h"

// TODO Can collapse this, but it wouldn't be a good pattern to use
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *date_tuple = dict_find(iter, DataKeyDate);
  if(date_tuple) {
    data_set_boolean_setting(DataKeyDate, (date_tuple->value->int8 == 1));
  }

  Tuple *anim_tuple = dict_find(iter, DataKeyAnimations);
  if(anim_tuple) {
    data_set_boolean_setting(DataKeyAnimations, (anim_tuple->value->int8 == 1));
  }

  Tuple *bt_tuple = dict_find(iter, DataKeyBTIndicator);
  if(bt_tuple) {
    data_set_boolean_setting(DataKeyBTIndicator, (bt_tuple->value->int8 == 1));
  }

  Tuple *hourly_tuple = dict_find(iter, DataKeyHourlyVibration);
  if(hourly_tuple) {
    data_set_boolean_setting(DataKeyHourlyVibration, (hourly_tuple->value->int8 == 1));
  }

  Tuple *sleep_tuple = dict_find(iter, DataKeySleep);
  if(sleep_tuple) {
    data_set_boolean_setting(DataKeySleep, (sleep_tuple->value->int8 == 1));
  }

#if defined(PBL_COLOR)
  Tuple *fg_tuple = dict_find(iter, DataKeyForegroundColor);
  if(fg_tuple) {
    data_set_foreground_color(GColorFromHEX(fg_tuple->value->int32));
  }

  Tuple *bg_tuple = dict_find(iter, DataKeyBackgroundColor);
  if(bg_tuple) {
    data_set_background_color(GColorFromHEX(bg_tuple->value->int32));
  }
#endif

  const bool delay = true;
  main_window_reload_config(delay);
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() { }
