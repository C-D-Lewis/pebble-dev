#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  PersistData *persist_data = data_get_persist_data();

  // Colors
  if (packet_contains_key(iter, MESSAGE_KEY_DAY_BG_COLOR)) {
    persist_data->day_bg_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_DAY_BG_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_DAY_BLOCK_COLOR)) {
    persist_data->day_block_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_DAY_BLOCK_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_DAY_SHADOW_COLOR)) {
    persist_data->day_shadow_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_DAY_SHADOW_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_DAY_VOID_COLOR)) {
    persist_data->day_void_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_DAY_VOID_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_NIGHT_BG_COLOR)) {
    persist_data->night_bg_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_NIGHT_BG_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_NIGHT_BLOCK_COLOR)) {
    persist_data->night_block_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_NIGHT_BLOCK_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_NIGHT_SHADOW_COLOR)) {
    persist_data->night_shadow_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_NIGHT_SHADOW_COLOR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_NIGHT_VOID_COLOR)) {
    persist_data->night_void_color = GColorFromHEX(
      packet_get_integer(iter, MESSAGE_KEY_NIGHT_VOID_COLOR)
    );
  }

  main_window_reload();
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() {}
