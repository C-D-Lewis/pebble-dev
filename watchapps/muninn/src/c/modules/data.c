#include "data.h"

// Persisted
static AppData s_app_data;
static SampleData s_sample_data;

// Not persisted
static char s_error_buff[64];

static void delete_all_data() {
  for (int i = 0; i < SK_Max; i += 1) persist_delete(i);
  wakeup_cancel_all();
  APP_LOG(APP_LOG_LEVEL_INFO, "!!! RESET ALL DATA !!!");
}

static void save_all() {
  status_t result = persist_write_data(SK_AppData, &s_app_data, sizeof(AppData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing app data: %d", (int)result);
    data_set_error("Error writing app data to storage");
  }

  result = persist_write_data(SK_SampleData, &s_sample_data, sizeof(SampleData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing sample data: %d", (int)result);
    data_set_error("Error writing sample data to storage");
  }
}

void data_reset_all() {
  delete_all_data();

  // Write defaults - some will be init'd when tracking begins
  s_app_data.last_sample_time = DATA_EMPTY;
  s_app_data.last_charge_perc = DATA_EMPTY;
  s_app_data.wakeup_id = DATA_EMPTY;
  s_app_data.was_plugged = true;
  s_app_data.seen_first_launch = false;
  s_app_data.vibe_on_sample = false;
  s_app_data.custom_alert_level = AL_OFF;
  s_app_data.ca_has_notified = false;

  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    s_sample_data.timestamps[i] = DATA_EMPTY;
    s_sample_data.values[i] = DATA_EMPTY;
  }

  save_all();
}

static void do_migrations() {
  // NUM_STORED_SAMPLES increased
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    if (s_sample_data.values[i] == 0) {
      // Due to expansion, new slots will be zeroed
      s_sample_data.timestamps[i] = DATA_EMPTY;
      s_sample_data.values[i] = DATA_EMPTY;
    }
  }

  // Future migrations go here
}

void data_init() {
#if defined(WIPE_ON_LAUNCH)
  delete_all_data();
#endif

#if defined(TEST_DATA)
  // Load test values for this launch
  s_app_data.last_sample_time = time(NULL) - (6 * SECONDS_PER_HOUR);
  s_app_data.last_charge_perc = 80;
  s_app_data.wakeup_id = time(NULL) + (12 * SECONDS_PER_HOUR);   // Won't be found
  s_app_data.was_plugged = false;
  s_app_data.seen_first_launch = true;
  s_app_data.vibe_on_sample = true;
  s_app_data.custom_alert_level = AL_20;
  s_app_data.ca_has_notified = false;

  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    // Going back i * six hours, on the hour
    s_sample_data.timestamps[i] = time(NULL) - ((i + 1) * WAKEUP_MOD_H * SECONDS_PER_HOUR);
    s_sample_data.values[i] = (i < 1) ? 4 : DATA_EMPTY;
  }
  return;
#endif

  // Never used, write defaults
  if (!persist_exists(SK_SampleData)) {
    data_reset_all();
  } else {
    // Load current data
    status_t result = persist_read_data(SK_AppData, &s_app_data, sizeof(AppData));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading app data: %d", (int)result);
      data_set_error("Error reading app data from storage");
      return;
    }

    result = persist_read_data(SK_SampleData, &s_sample_data, sizeof(SampleData));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading sample data: %d", (int)result);
      data_set_error("Error reading sample data from storage");
      return;
    }

    do_migrations();
  }

  data_log_state();
}

void data_deinit() {
#if !defined(TEST_DATA) || (defined(TEST_DATA) && defined(SAVE_TEST_DATA))
  save_all();
#endif
}

void data_log_state() {
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "D: %d | W: %d | B: %d | P: %s | H: %d %d %d | A: %d",
    s_app_data.last_sample_time, s_app_data.wakeup_id,
    s_app_data.last_charge_perc, s_app_data.was_plugged ? "t": "f",
    s_sample_data.values[0], s_sample_data.values[1],
    s_sample_data.values[2], s_app_data.ca_has_notified ? 1 : 0
  );
}

void data_initial_update() {
  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;
  const time_t now = time(NULL);

  // We set these when enabled to rely on the time diff and zero-diff mechanisms
  data_set_last_charge_perc(charge_percent);
  data_set_last_sample_time(now);

  // Assume this is discharge start until events change that
  data_set_was_plugged(is_plugged);
}

void data_push_sample_value(int v) {
  // Time of the new sample
  const time_t now = time(NULL);

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

  // At least two samples to average
  if (counted < 2) return DATA_EMPTY;

  return acc / counted;
}

int data_calculate_days_remaining() {
  int rate = data_get_history_avg_rate();
  int charge_perc = data_get_last_charge_perc();

  if (rate == DATA_EMPTY || charge_perc == DATA_EMPTY) return DATA_EMPTY;

  if (rate <= 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Somehow rate was <= 0 (negative)");
    APP_LOG(APP_LOG_LEVEL_INFO, "rate: %d charge_perc: %d", rate, charge_perc);
    return DATA_EMPTY;
  }


  return charge_perc / rate;
}

int data_get_last_sample_time() {
  return s_app_data.last_sample_time;
}

void data_set_last_sample_time(int time) {
  s_app_data.last_sample_time = time;
}

int data_get_last_charge_perc() {
  return s_app_data.last_charge_perc;
}

void data_set_last_charge_perc(int perc) {
  s_app_data.last_charge_perc = perc;
}

int data_get_wakeup_id() {
  return s_app_data.wakeup_id;
}

void data_set_wakeup_id(int id) {
  s_app_data.wakeup_id = id;
}

bool data_get_was_plugged() {
  return s_app_data.was_plugged;
}

void data_set_was_plugged(bool b) {
  s_app_data.was_plugged = b;
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
  s_app_data.seen_first_launch = true;
}

bool data_get_seen_first_launch() {
  return s_app_data.seen_first_launch;
}

bool data_get_vibe_on_sample() {
  return s_app_data.vibe_on_sample;
}

void data_set_vibe_on_sample(bool v) {
  s_app_data.vibe_on_sample = v;
}

int data_get_custom_alert_level() {
  return s_app_data.custom_alert_level;
}

void data_cycle_custom_alert_level() {
  switch (s_app_data.custom_alert_level) {
    case AL_OFF:
      s_app_data.custom_alert_level = AL_50;
      break;
    case AL_50:
      s_app_data.custom_alert_level = AL_20;
      break;
    case AL_20:
      s_app_data.custom_alert_level = AL_10;
      break;
    case AL_10:
    default:
      s_app_data.custom_alert_level = AL_OFF;
      break;
  }

  // If changed to less than current, don't skip
  data_set_ca_has_notified(false);
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

bool data_get_ca_has_notified() {
  return s_app_data.ca_has_notified;
}

void data_set_ca_has_notified(bool notified) {
  s_app_data.ca_has_notified = notified;
}
