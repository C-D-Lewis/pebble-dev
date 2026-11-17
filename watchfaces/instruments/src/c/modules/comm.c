#include "comm.h"

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  PersistData *persist_data = data_get_persist_data();
  AppState *app_state = data_get_app_state();

  // Ready
  if (packet_contains_key(iter, MESSAGE_KEY_READY)) {
    comm_request_data();
    return;
  }

  // Weather
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_TEMP)) {
    persist_data->current_temp = packet_get_integer(iter, MESSAGE_KEY_CURRENT_TEMP);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CURRENT_CODE)) {
    persist_data->current_code = packet_get_integer(iter, MESSAGE_KEY_CURRENT_CODE);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_SUNRISE)) {
    snprintf(persist_data->sunrise,
      sizeof(persist_data->sunrise),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_SUNRISE)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_SUNSET)) {
    snprintf(persist_data->sunset,
      sizeof(persist_data->sunset),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_SUNSET)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_PRECIP_ARR)) {
    snprintf(
      persist_data->precip_arr,
      sizeof(persist_data->precip_arr),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_PRECIP_ARR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CODE_ARR)) {
    snprintf(
      persist_data->code_arr,
      sizeof(persist_data->code_arr),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CODE_ARR)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_TEMP_LOW)) {
    persist_data->temp_low = packet_get_integer(iter, MESSAGE_KEY_TEMP_LOW);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_TEMP_HIGH)) {
    persist_data->temp_high = packet_get_integer(iter, MESSAGE_KEY_TEMP_HIGH);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_UV_INDEX_MAX)) {
    persist_data->uv_index_max = packet_get_integer(iter, MESSAGE_KEY_UV_INDEX_MAX);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_MOON_PHASE)) {
    persist_data->moon_phase = packet_get_integer(iter, MESSAGE_KEY_MOON_PHASE);
  }

  // Config
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_TEMP_UNIT)) {
    snprintf(
      persist_data->temp_unit,
      sizeof(persist_data->temp_unit),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_TEMP_UNIT)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_THEME)) {
    snprintf(
      persist_data->theme,
      sizeof(persist_data->theme),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_THEME)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_SLOT_1)) {
    snprintf(
      persist_data->slot_1,
      sizeof(persist_data->slot_1),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_SLOT_1)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_SLOT_2)) {
    snprintf(
      persist_data->slot_2,
      sizeof(persist_data->slot_2),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_SLOT_2)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_SLOT_3)) {
    snprintf(
      persist_data->slot_3,
      sizeof(persist_data->slot_3),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_SLOT_3)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_SLOT_4)) {
    snprintf(
      persist_data->slot_4,
      sizeof(persist_data->slot_4),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_SLOT_4)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_SLOT_5)) {
    snprintf(
      persist_data->slot_5,
      sizeof(persist_data->slot_5),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_SLOT_5)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_SLOT_6)) {
    snprintf(
      persist_data->slot_6,
      sizeof(persist_data->slot_6),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_SLOT_6)
    );
  }
  if (packet_contains_key(iter, MESSAGE_KEY_CONFIG_STEP_GOAL)) {
    snprintf(
      persist_data->step_goal,
      sizeof(persist_data->step_goal),
      "%s",
      packet_get_string(iter, MESSAGE_KEY_CONFIG_STEP_GOAL)
    );
  }

  // Other data
  if (packet_contains_key(iter, MESSAGE_KEY_PHONE_BATTERY)) {
    persist_data->phone_battery = packet_get_integer(iter, MESSAGE_KEY_PHONE_BATTERY);
  }
  if (packet_contains_key(iter, MESSAGE_KEY_DATA_ERROR)) {
    persist_data->current_code = DATA_ERROR;
  }

  app_state->data_updated = true;
  main_window_reload();
}

void comm_request_data() {
  PersistData *persist_data = data_get_persist_data();

  if (packet_begin()) {
    packet_put_integer(MESSAGE_KEY_REQUEST_WEATHER, 1);
    packet_put_string(MESSAGE_KEY_REQUEST_SLOT_1, persist_data->slot_1);
    packet_put_string(MESSAGE_KEY_REQUEST_SLOT_2, persist_data->slot_2);
    packet_put_string(MESSAGE_KEY_REQUEST_SLOT_3, persist_data->slot_3);
    packet_put_string(MESSAGE_KEY_REQUEST_SLOT_4, persist_data->slot_4);
    packet_put_string(MESSAGE_KEY_REQUEST_SLOT_5, persist_data->slot_5);
    packet_put_string(MESSAGE_KEY_REQUEST_SLOT_6, persist_data->slot_6);
    packet_send(NULL);
  }
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  packet_init();

  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox, outbox);
}

void comm_deinit() {}
