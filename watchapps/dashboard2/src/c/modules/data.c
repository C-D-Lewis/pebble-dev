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
  s_app_state.sync_state = SyncStateInitial;

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

#ifdef USE_TEST_DATA
void data_test_data_handler() {
  s_app_state.sync_state = SyncStateSuccess;
  // Set higher than COMPAT_PROTOCOL_VERSION to simulate incompatability
  s_app_state.compat_protocol_version = 2;

  snprintf(
    s_app_state.toggle_order,
    sizeof(s_app_state.toggle_order),
    
    "100000000000000000"
  );
  snprintf(
    s_app_state.device_name,
    sizeof(s_app_state.device_name),
    "Test Device"
  );
  snprintf(
    s_app_state.battery_perc,
    sizeof(s_app_state.battery_perc),
    "89"
  );
  snprintf(
    s_app_state.free_space,
    sizeof(s_app_state.free_space),
    "117.38 GB"
  );
  snprintf(
    s_app_state.free_space_perc,
    sizeof(s_app_state.free_space_perc),
    "76"
  );

  main_window_update();
}
#endif

int data_get_toggles_length() {
  int count = 0;

  // Go through [type, state] pairs until the last is rached
  for (int i = 0; i < TOGGLES_STRLEN - 1; i += 2) {
    if (s_app_state.toggle_order[i] == '0') break;
    count++;
  }
  return count;
}
