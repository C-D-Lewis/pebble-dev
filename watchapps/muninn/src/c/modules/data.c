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

  // Init all fields in Sample struct
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    Sample *s = &s_sample_data.samples[i];
    s->timestamp = DATA_EMPTY;
    s->charge_perc = DATA_EMPTY;
    s->last_sample_time = DATA_EMPTY;
    s->last_charge_perc = DATA_EMPTY;
    s->time_diff = DATA_EMPTY;
    s->charge_diff = DATA_EMPTY;
    s->perc_per_day = DATA_EMPTY;
  }

  save_all();
}

static void do_migrations() {
  // NUM_STORED_SAMPLES increased - 4 ts + 4 values went into 8 timestamps... hard to migrate

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

  const time_t now = time(NULL);
  const int gap_perc = 4;

  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    Sample *s = &s_sample_data.samples[i];

    // Hopefully this is backwards from now, every six hours, 4% estimate
    s->timestamp = now - ((i + 1) * WAKEUP_MOD_H * SECONDS_PER_HOUR);
    s->perc_per_day = gap_perc;
    s->charge_perc = s_app_data.last_charge_perc - (i * gap_perc);
    s->last_sample_time = now - ((i + 2) * WAKEUP_MOD_H * SECONDS_PER_HOUR);
    s->last_charge_perc = s_app_data.last_charge_perc - ((i + 1) * gap_perc);
    s->time_diff = WAKEUP_MOD_H * SECONDS_PER_HOUR;
    s->charge_diff = 1;
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
    s_sample_data.samples[0].perc_per_day, s_sample_data.samples[1].perc_per_day,
    s_sample_data.samples[2].perc_per_day, s_app_data.ca_has_notified ? 1 : 0
  );

#if defined(DEBUG_LOGS)
  // Sample history
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    Sample *s = &s_sample_data.samples[i];
    APP_LOG(
      APP_LOG_LEVEL_INFO,
      "%d | t:%d c:%d ls:%d lc:%d td:%d cd:%d ppd:%d",
      i,
      (int)s->timestamp,
      (int)s->charge_perc,
      (int)s->last_sample_time,
      (int)s->last_charge_perc,
      (int)s->time_diff,
      (int)s->charge_diff,
      (int)s->perc_per_day
    );
  }
#endif
}

void data_activation_update() {
  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;
  const time_t now = time(NULL);

  data_set_was_plugged(is_plugged);

  // Set this as a time period start, so we can at least begin something now
  data_set_last_sample_time(now);

  // If a fairly close amount of time to the full period is left, capture the level
  // This should help with initial under-estimation of the discharge rate
  if (util_hours_until_next_interval() >= (3 * WAKEUP_MOD_H) / 4) {
    data_set_last_charge_perc(charge_percent);
  }
}

void data_push_sample(int charge_perc, int last_sample_time, int last_charge_perc, int time_diff, int charge_diff, int perc_per_day) {
  // Shift right previous samples
  for (int i = NUM_STORED_SAMPLES - 1; i > 0; i--) {
    s_sample_data.samples[i] = s_sample_data.samples[i - 1];
  }

  Sample *s = &s_sample_data.samples[0];
  s->timestamp = time(NULL);
  s->charge_perc = charge_perc;
  s->last_sample_time = last_charge_perc;
  s->last_charge_perc = last_charge_perc;
  s->time_diff = time_diff;
  s->charge_diff = charge_diff;
  s->perc_per_day = perc_per_day;
}

/**
 * This _should_ place twice as much emphasis on the most recent half of values,
 * so recent changes in watchface or activity are more quickly reflected.
 */
int data_calculate_avg_discharge_rate() {
  SampleData *data = data_get_sample_data();
  const int count = data_get_samples_count();
  if (count < MIN_SAMPLES) return DATA_EMPTY;

  const int half_index = (count + 1) / 2;
  APP_LOG(APP_LOG_LEVEL_INFO, "avg: count: %d half_index: %d", count, half_index);

  int result_x2 = 0;
  int weight_x2 = 0;
  int seen = 0;

  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    int v = data->samples[i].perc_per_day;
#if defined(DEBUG_LOGS)
#endif
    if (v == DATA_EMPTY) continue;

    seen++;
    if (seen <= half_index) {
      result_x2 += v * 2; // full weight -> doubled
      weight_x2 += 2;
#if defined(DEBUG_LOGS)
      APP_LOG(APP_LOG_LEVEL_INFO, "avg: i:%d x2 %d -> %d (seen: %d)", i, v, 2 * v, seen);
#endif
    } else {
      result_x2 += v * 1; // half weight -> single
      weight_x2 += 1;
#if defined(DEBUG_LOGS)
      APP_LOG(APP_LOG_LEVEL_INFO, "avg: i:%d x1 %d (seen: %d)", i, v, seen);
#endif
    }
  }

  if (weight_x2 == 0) return DATA_EMPTY;
  const int avg = result_x2 / weight_x2;
#if defined(DEBUG_LOGS)
  APP_LOG(APP_LOG_LEVEL_INFO, "avg: result_x2 %d / weight_x2 %d => %d\n---", result_x2, weight_x2, avg);
#endif
  return avg;
}

int data_calculate_days_remaining() {
  int rate = data_calculate_avg_discharge_rate();
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
    if (s_sample_data.samples[i].perc_per_day != DATA_EMPTY) {
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
