#include "data.h"

static PersistData s_persist_data;
static AppState s_app_state;

static void save() {
  status_t result = persist_write_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "data w err %d", (int)result);
  }
  if (result < (int)sizeof(PersistData)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "data w trunc");
  }
}

// These include default settings for new app users
static void init_data_fields() {
  // s_persist_data.last_sample_time = STATUS_EMPTY;
}

void data_init() {
  // Init AppState emphemeral fields
  // s_app_state.sync_count = STATUS_EMPTY;

  // Never used, write defaults
  if (!persist_exists(SK_PersistData)) {
    init_data_fields();
  } else {
    // Load current data
    status_t result = persist_read_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "app read fail %d", (int)result);
      return;
    }
  }
}

void data_deinit() {
  save();
}

PersistData* data_get_persist_data() {
  return &s_persist_data;
}

AppState* data_get_app_state() {
  return &s_app_state;
}
