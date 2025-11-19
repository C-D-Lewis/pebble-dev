#include "data.h"

// Persisted
static int s_discharge_start_time;
static int s_last_charge_perc;
static int s_wakeup_id;
static bool s_was_plugged;
static bool s_seen_first_launch;
static bool s_vibe_on_sample;
static int s_custom_alert_level;
static SampleData s_sample_data;

// Not persisted
static char s_error_buff[64];

static void delete_all_data() {
  for (int i = 0; i < SK_Max; i += 1) persist_delete(i);
  wakeup_cancel_all();
  APP_LOG(APP_LOG_LEVEL_INFO, "!!! RESET ALL DATA !!!");
}

static void save_all() {
  persist_write_int(SK_DischargeStartTime, s_discharge_start_time);
  persist_write_int(SK_LastChargePerc, s_last_charge_perc);
  persist_write_int(SK_WakeupId, s_wakeup_id);
  persist_write_bool(SK_WasPlugged, s_was_plugged);
  persist_write_bool(SK_SeenFirstLaunch, s_seen_first_launch);
  persist_write_bool(SK_VibeOnSample, s_vibe_on_sample);
  persist_write_int(SK_CustomAlertLevel, s_custom_alert_level);

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
  s_last_charge_perc = DATA_EMPTY;
  s_wakeup_id = DATA_EMPTY;
  s_was_plugged = true;
  s_seen_first_launch = true; // Special case
  s_vibe_on_sample = false;
  s_custom_alert_level = AL_OFF;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_sample_data.values[i] = DATA_EMPTY;
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
  s_last_charge_perc = 50;
  s_wakeup_id = time(NULL) - (12 * SECONDS_PER_HOUR);   // Won't be found
  s_was_plugged = false;
  s_seen_first_launch = true;
  s_vibe_on_sample = true;
  s_custom_alert_level = AL_20;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_sample_data.timestamps[i] = time(NULL) - ((i + 1) * SECONDS_PER_DAY);
    s_sample_data.values[i] = (i < 3) ? 5 : DATA_EMPTY;
  }
  return;
#endif

  // Never used, write defaults
  if (!persist_exists(SK_SampleData)) {
    data_reset_all();
  } else {
    // Load current data for foreground display
    s_discharge_start_time = persist_read_int(SK_DischargeStartTime);
    s_last_charge_perc = persist_read_int(SK_LastChargePerc);
    s_wakeup_id = persist_read_int(SK_WakeupId);
    s_was_plugged = persist_read_bool(SK_WasPlugged);
    s_seen_first_launch = persist_read_bool(SK_SeenFirstLaunch);
    s_vibe_on_sample = persist_read_bool(SK_VibeOnSample);
    s_custom_alert_level = persist_read_int(SK_CustomAlertLevel);
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
#if !defined(TEST_DATA) || (defined(TEST_DATA) && defined(SAVE_TEST_DATA))
  save_all();
#endif
}

void data_log_state() {
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "S: %d, W: %d, B: %d, P: %s",
    s_discharge_start_time, s_wakeup_id, s_last_charge_perc, s_was_plugged ? "true": "false"
  );
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "History: %d %d %d %d %d %d",
    s_sample_data.values[0],
    s_sample_data.values[1],
    s_sample_data.values[2],
    s_sample_data.values[3],
    s_sample_data.values[4],
    s_sample_data.values[5]
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
}

void data_push_sample_value(int v) {
  // Time of the new sample
  time_t now = time(NULL);

  // Shift right
  for (int i = NUM_STORED_SAMPLES - 1; i > 0; i--) {
    s_sample_data.timestamps[i] = s_sample_data.timestamps[i - 1];
    s_sample_data.values[i] = s_sample_data.values[i - 1];
  }

  s_sample_data.timestamps[0] = now;
  s_sample_data.values[0] = v;
}

int data_get_history_avg_rate() {
  SampleData *data = data_get_sample_data();

  int acc = 0;
  int counted = 0;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    if (data->values[i] == DATA_EMPTY) continue;
    
    acc += data->values[i];
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
  return s_discharge_start_time;
}

void data_set_discharge_start_time(int time) {
  s_discharge_start_time = time;
}

int data_get_last_charge_perc() {
  return s_last_charge_perc;
}

void data_set_last_charge_perc(int perc) {
  s_last_charge_perc = perc;
}

int data_get_wakeup_id() {
  return s_wakeup_id;
}

void data_set_wakeup_id(int id) {
  s_wakeup_id = id;
}

bool data_get_was_plugged() {
  return s_was_plugged;
}

void data_set_was_plugged(bool b) {
  s_was_plugged = b;
}

SampleData* data_get_sample_data() {
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

void data_set_seen_first_launch() {
  s_seen_first_launch = true;
}

bool data_get_seen_first_launch() {
  return s_seen_first_launch;
}

bool data_get_vibe_on_sample() {
  return s_vibe_on_sample;
}

void data_set_vibe_on_sample(bool v) {
  s_vibe_on_sample = v;
}

int data_get_custom_alert_level() {
  return s_custom_alert_level;
}

void data_cycle_custom_alert_level() {
  switch (s_custom_alert_level) {
    case AL_OFF:
      s_custom_alert_level = AL_50;
      break;
    case AL_50:
      s_custom_alert_level = AL_20;
      break;
    case AL_20:
      s_custom_alert_level = AL_10;
      break;
    case AL_10:
    default:
      s_custom_alert_level = AL_OFF;
      break;
  }
}

int data_get_samples_count() {
  int count = 0;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    if (s_sample_data.values[i] != DATA_EMPTY) {
      count++;
    }
  }
  return count;
}
