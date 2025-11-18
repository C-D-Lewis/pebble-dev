#include "data.h"

// Persisted
static int s_discharge_start_time;
static int s_last_update_time;
static int s_last_charge_perc;
static int s_wakeup_id;
static bool s_was_plugged;
static SampleData s_sample_data;

#if defined(TEST_DATA)
static SampleData s_test_data;
#endif

// Not persisted
static char s_error_buff[64];

static void delete_all_data() {
  for (int i = 0; i < SK_Max; i += 1) persist_delete(i);
  wakeup_cancel_all();
  APP_LOG(APP_LOG_LEVEL_INFO, "!!! RESET ALL DATA !!!");
}

static void save_all() {
  persist_write_int(SK_DischargeStartTime, s_discharge_start_time);
  persist_write_int(SK_LastUpdateTime, s_last_update_time);
  persist_write_int(SK_LastChargePerc, s_last_charge_perc);
  persist_write_int(SK_WakeupId, s_wakeup_id);
  persist_write_bool(SK_WasPlugged, s_was_plugged);

  status_t result = persist_write_data(SK_SampleData, &s_sample_data, sizeof(SampleData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing sample data: %d", (int)result);
    data_set_error("Error writing data to storage");
  }
}

void data_reset_all() {
  delete_all_data();

  // Write defaults - some will be init'd when tracking begins
  s_discharge_start_time = DATA_EMPTY;
  s_last_update_time = DATA_EMPTY;
  s_last_charge_perc = DATA_EMPTY;
  s_wakeup_id = DATA_EMPTY;
  s_was_plugged = true;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_sample_data.history[i] = DATA_EMPTY;
  }

  save_all();
}

void data_init() {
#if defined(WIPE_ON_LAUNCH)
  delete_all_data();
#endif

#if defined(TEST_DATA)
  // Load test values for this launch
  s_discharge_start_time = time(NULL) - (6 * SECONDS_PER_HOUR);
  s_last_update_time = time(NULL) - SECONDS_PER_HOUR;
  s_last_charge_perc = 50;
  s_wakeup_id = 12345;
  s_was_plugged = false;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_test_data.history[i] = 4;
  }
  return;
#endif

#if defined(LOAD_DATA)
  // Load some sample data into persist for testing
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_sample_data.history[i] = (i == 0) ? 4 : DATA_EMPTY;
  }
  persist_write_data(SK_SampleData, &s_sample_data, sizeof(SampleData));
#endif

  // Never used, write defaults
  if (!persist_exists(SK_SampleData)) {
    data_reset_all();
  } else {
    // Load current data for foreground display
    s_discharge_start_time = persist_read_int(SK_DischargeStartTime);
    s_last_update_time = persist_read_int(SK_LastUpdateTime);
    s_last_charge_perc = persist_read_int(SK_LastChargePerc);
    s_wakeup_id = persist_read_int(SK_WakeupId);
    s_was_plugged = persist_read_bool(SK_WasPlugged);
    status_t result = persist_read_data(SK_SampleData, &s_sample_data, sizeof(SampleData));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading sample data: %d", (int)result);
      data_set_error("Error reading data from storage");
      return;
    }

    data_log_state();
  }
}

void data_deinit() {
  save_all();
}

void data_log_state() {
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "S: %d, U: %d, W: %d, B: %d, P: %s",
    s_discharge_start_time, s_last_update_time, s_wakeup_id, s_last_charge_perc,
    s_was_plugged ? "true": "false"
  );
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "History: %d %d %d %d %d %d",
    s_sample_data.history[0],
    s_sample_data.history[1],
    s_sample_data.history[2],
    s_sample_data.history[3],
    s_sample_data.history[4],
    s_sample_data.history[5]
  );
}

void data_initial_sample() {
  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;

  data_set_was_plugged(is_plugged);
  data_set_last_charge_perc(charge_percent);

  const time_t now = time(NULL);
  data_set_discharge_start_time(now);

  // Don't set update_time - less than the period doesn't give good first estimates
  // data_set_last_update_time(now);
}

void data_push_sample_value(int v) {
  // Shift right
  for (int i = NUM_STORED_SAMPLES - 1; i > 0; i--) {
    s_sample_data.history[i] = s_sample_data.history[i - 1];
  }
  s_sample_data.history[0] = v;
}

int data_get_history_avg_rate() {
  SampleData *data = data_get_sample_data();

  int acc = 0;
  int counted = 0;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    if (data->history[i] == DATA_EMPTY) continue;
    
    acc += data->history[i];
    counted++;
  }

  if (counted == 0) return DATA_EMPTY;
  return acc / counted;
}

int data_calculate_days_remaining() {
  int rate = data_get_history_avg_rate();
  int remaining = data_get_last_charge_perc();

  if (rate == DATA_EMPTY || remaining == DATA_EMPTY) return DATA_EMPTY;
  if (rate <= 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Somehow rate was negative");
    APP_LOG(APP_LOG_LEVEL_INFO, "rate: %d remaining: %d", rate, remaining);
    return DATA_EMPTY;
  }

  return remaining / rate;
}

int data_get_discharge_start_time() {
#if defined(TEST_DATA)
  return time(NULL) - (6 * SECONDS_PER_HOUR);
#endif
  return s_discharge_start_time;
}

void data_set_discharge_start_time(int time) {
  s_discharge_start_time = time;
}

int data_get_last_update_time() {
#if defined(TEST_DATA)
  return time(NULL) - SECONDS_PER_HOUR;
#endif
  return s_last_update_time;
}

void data_set_last_update_time(int time) {
  s_last_update_time = time;
}

int data_get_last_charge_perc() {
#if defined(TEST_DATA)
  return 50;
#endif
  return s_last_charge_perc;
}

void data_set_last_charge_perc(int perc) {
  s_last_charge_perc = perc;
}

int data_get_wakeup_id() {
  // Probably can't fake this
#if defined(TEST_DATA)
  return time(NULL) + (6 * SECONDS_PER_HOUR);
#endif
  return s_wakeup_id;
}

void data_set_wakeup_id(int id) {
  s_wakeup_id = id;
}

bool data_get_was_plugged() {
#if defined(TEST_DATA)
  return false;
#endif
  return s_was_plugged;
}

void data_set_was_plugged(bool b) {
  s_was_plugged = b;
}

SampleData* data_get_sample_data() {
#if defined(TEST_DATA)
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_test_data.history[i] = 5;
  }
  return &s_test_data;
#endif
  return &s_sample_data;
}

void data_set_error(char *err) {
  snprintf(s_error_buff, sizeof(s_error_buff), "Error: %s", err);
  alert_window_push(
    RESOURCE_ID_ASLEEP,
    data_get_error(),
    true,
    false
  );
}

char* data_get_error() {
  return &s_error_buff[0];
}
