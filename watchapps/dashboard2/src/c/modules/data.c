#include "data.h"

static AppState s_app_state;

void data_init() {
  // Init AppState emphemeral fields
  s_app_state.sync_state = SyncStateInitial;
}

void data_deinit() {
}

AppState* data_get_app_state() {
  return &s_app_state;
}

#ifdef USE_TEST_DATA
void data_test_data_handler() {
  s_app_state.sync_state = SyncStateSuccess;
  // Set higher than COMPAT_PROTOCOL_VERSION to simulate incompatability
  s_app_state.compat_protocol_version = COMPAT_PROTOCOL_VERSION;

  snprintf(
    s_app_state.toggle_order,
    sizeof(s_app_state.toggle_order),
    
    "10"
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
  for (int i = 0; i < TOGGLES_STRLEN - 1; i += TOGGLE_FMT_LEN) {
    if (s_app_state.toggle_order[i] == '0') break;
    count++;
  }
  return count;
}

bool data_is_toggle_active(int index) {
  if (index < 0 || index >= data_get_toggles_length()) return false;

  // Get the state character for this toggle
  const char c = s_app_state.toggle_order[(index * TOGGLE_FMT_LEN) + 1];
  return c == TOGGLE_STATE_ON;
}
