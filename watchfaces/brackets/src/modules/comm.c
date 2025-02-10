#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  for(int i = 0; i < DATA_NUM_BOOLEANS + DATA_NUM_COLORS; i++) {
    Tuple *t = dict_find(iter, i);
    if(t) {
      int key = t->key;
      switch(key) {
        case AppKeyBatteryMeter:
        case AppKeyBluetoothAlert:
        case AppKeyDashedLine:
        case AppKeySecondTick:
          data_set_boolean(key, strcmp(t->value->cstring, "true") == 0);
          break;

        case AppKeyBackgroundColor:
        case AppKeyDateColor:
        case AppKeyTimeColor:
        case AppKeyBracketColor:
        case AppKeyLineColor:
        case AppKeyComplicationColor:
          data_set_color(key, (GColor){ .argb = t->value->int32 });
          printf("%d got %d", key, (int)t->value->int32);
          break;
      }
    }
  }

  // Exit to reload
  main_window_reload();
}

void comm_init() {
  app_message_register_inbox_received(inbox_received_handler);

  const int inbox_size = 512;
  const int outbox_size = 64;
  app_message_open(inbox_size, outbox_size);
}
