#include "comm.h"

#include <pebble-packet/pebble-packet.h>

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
#if defined(PBL_COLOR)
  // Color settings
  if(packet_contains_key(iter, AppKeyColorBackground)) {
    data_set_color(ColorBackground, GColorFromHEX(packet_get_integer(iter, AppKeyColorBackground)));
  }

  if(packet_contains_key(iter, AppKeyColorSides)) {
    data_set_color(ColorSides, GColorFromHEX(packet_get_integer(iter, AppKeyColorSides)));
  }

  if(packet_contains_key(iter, AppKeyColorFace)) {
    data_set_color(ColorFace, GColorFromHEX(packet_get_integer(iter, AppKeyColorFace)));
  }
#endif

  // Other settings
  if(packet_contains_key(iter, AppKeyAnimations)) {
    data_set_animations(packet_get_boolean(iter, AppKeyAnimations));
  }

  if(packet_contains_key(iter, AppKeyBluetooth)) {
    data_set_bluetooth_alert(packet_get_boolean(iter, AppKeyBluetooth));
  }

  main_reload_config();
}

void comm_init(int inbox, int outbox) {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}
