#include "data.h"

static PersistData s_persist_data;
static AppState s_app_state;

static void save_all() {
  status_t result = persist_write_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing persist data: %d", (int)result);
  }
}

void data_init() {
  if (persist_exists(SK_PersistData)) {
    // Load existing data
    status_t result = persist_read_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading persist data: %d", (int)result);
    }
  } else {
    // Defaults
    s_persist_data.second_tick = true;
    s_persist_data.battery_and_bluetooth = true;
    snprintf(
      s_persist_data.complication_type,
      sizeof(s_persist_data.complication_type),
      COMPLICATION_TYPE_WEATHER
    );
    s_persist_data.color_background = GColorWhite;
    s_persist_data.color_brackets = GColorBlack;
    s_persist_data.color_datetime = GColorBlack;
    s_persist_data.color_complications = GColorBlack;
    
    save_all();
  }
}

void data_deinit() {
  save_all();
}

PersistData* data_get_persist_data() {
  return &s_persist_data;
}

AppState* data_get_app_state() {
  return &s_app_state;
}
