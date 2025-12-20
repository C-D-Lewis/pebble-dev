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
  s_app_data.last_sample_time = STATUS_EMPTY;
  s_app_data.last_charge_perc = STATUS_EMPTY;
  s_app_data.wakeup_id = STATUS_EMPTY;
  s_app_data.seen_first_launch = false;
  s_app_data.vibe_on_sample = false;
  s_app_data.custom_alert_level = AL_OFF;
  s_app_data.ca_has_notified = false;
  s_app_data.push_timeline_pins = false;
  s_app_data.elevated_rate_alert = false;

  // Init all fields in Samples struct
  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_sample_data.samples[i];
    s->timestamp = STATUS_EMPTY;
    s->charge_perc = STATUS_EMPTY;
    s->last_sample_time = STATUS_EMPTY;
    s->last_charge_perc = STATUS_EMPTY;
    s->time_diff = STATUS_EMPTY;
    s->charge_diff = STATUS_EMPTY;
    s->result = STATUS_EMPTY;
  }

  save_all();
}

static void do_migrations() {
  // Future migrations go here
}

#if defined(USE_TEST_DATA)
static int result_from_gap(int gap) {
  if (gap < 0) {
    return STATUS_CHARGED;
  } else if (gap == 0) {
    return STATUS_NO_CHANGE;
  } else {
    return (gap * 4);
  }
}
#endif

void data_init() {
#if defined(USE_TEST_DATA)
  //
  // Test data scenarios
  //
  // 1 - Arbitrary scenario
  // const int changes[NUM_SAMPLES] = {30, 30, 30, 30, 30, 30, 30, 30};
  //
  // 2 - Test case: Should show 10 days at 8% per day (from 80%)
  // const int changes[NUM_SAMPLES] = {2, 2, 2, 2, 2, 2, 2, 2};
  //
  // 3 - Test case: Should show 11 days at 7% (two other events are ignored)
  //     Note: includes the two special statuses
  const int changes[NUM_SAMPLES] = {2, 2, 2, 2, -20, 2, 0, 2};
  //
  // 4 - Test case: Should show 6 days at 12% per day (from 80%)
  // const int changes[NUM_SAMPLES] = {3, 3, 3, 3, 3, 3, 3, 3};
  //
  // 5 - Test case: User submitted scenario for ~12% rate (not 40!)
  //     Note: the code to set up fake scenarios won't show the correct overlapping periods
  // const int changes[NUM_SAMPLES] = {0, 10, 0, 0, 10, 0, 10, 0};
  //
  // 6 - Test case: Last estimation was a significant uptick
  // const int changes[NUM_SAMPLES] = {20, 10, 0, 0, 10, 0, 10, 0};

  int total_change = 0;
  for(int i = 0; i < NUM_SAMPLES; i++) {
    total_change += changes[i];
  }
  const int interval_s = WAKEUP_MOD_H * SECONDS_PER_HOUR;

  // Use the most recent interval time as a base with 0 minutes
  const time_t now = time(NULL);
  struct tm *ts_info = localtime(&now);
  ts_info->tm_hour -= (ts_info->tm_hour % WAKEUP_MOD_H);
  ts_info->tm_min = 0;
  ts_info->tm_sec = 5;
  const time_t base = mktime(ts_info);

  // Load test values for this launch
  // TODO: Use the array's newest value here?
  s_app_data.last_sample_time = base;
  s_app_data.last_charge_perc = 80;  // Agrees with emulator
  s_app_data.wakeup_id = base + (12 * SECONDS_PER_HOUR);   // Won't be found
  s_app_data.seen_first_launch = true;
  s_app_data.vibe_on_sample = true;
  s_app_data.custom_alert_level = AL_20;
  s_app_data.ca_has_notified = false;
  s_app_data.elevated_rate_alert = false;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_sample_data.samples[i];
    const int gap = changes[i];

    if (i == 0) {
      s->timestamp = base;
      s->charge_perc = s_app_data.last_charge_perc;
      s->last_sample_time = base - interval_s;
      s->last_charge_perc = s->charge_perc + gap;
      s->charge_diff = gap;
      s->time_diff = interval_s;
      s->result = result_from_gap(gap);
      continue;
    }

    // Use the previous in the array which is more recent
    Sample *newer_s = &s_sample_data.samples[i - 1];

    s->timestamp = newer_s->last_sample_time;
    s->charge_perc = newer_s->last_charge_perc;
    s->last_sample_time = s->timestamp - interval_s;
    s->last_charge_perc = s->charge_perc + gap;
    s->charge_diff = gap;
    s->time_diff = interval_s;
    s->result = result_from_gap(gap);
  }

  data_log_state();
  return;
#endif

  // Never used, write defaults
  if (!persist_exists(SK_AppData)) {
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
#if !defined(USE_TEST_DATA) || (defined(USE_TEST_DATA) && defined(SAVE_TEST_DATA))
  save_all();
#endif
}

void data_log_state() {
#if defined(LOG_STATE)
  time_t wakeup_ts = STATUS_EMPTY;
  const bool is_enabled = util_is_valid(s_app_data.wakeup_id);
  if (is_enabled) {
    wakeup_query(s_app_data.wakeup_id, &wakeup_ts);
  }

  // App state
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "lst:%d lcp:%d wi:%d wts:%d sfl:%s vos:%s cal:%d chn:%s",
    s_app_data.last_sample_time,
    s_app_data.last_charge_perc,
    s_app_data.wakeup_id,
    (int)wakeup_ts,
    s_app_data.seen_first_launch == 1 ? "T" : "F",
    s_app_data.vibe_on_sample == 1 ? "T" : "F",
    s_app_data.custom_alert_level,
    s_app_data.ca_has_notified == 1 ? "T" : "F"
  );

  // Sample history
  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_sample_data.samples[i];
    APP_LOG(
      APP_LOG_LEVEL_INFO,
      "%d | t:%d -> %d (%d) | c:%d -> %d (%d) | ~%d",
      i,
      (int)s->last_sample_time,
      (int)s->timestamp,
      (int)s->time_diff,
      (int)s->last_charge_perc,
      (int)s->charge_perc,
      (int)s->charge_diff,
      (int)s->result
    );
  }
#endif
}

void data_push_sample(int charge_perc, int last_sample_time, int last_charge_perc, int time_diff, int charge_diff, int result) {
  // Shift right previous samples
  for (int i = NUM_SAMPLES - 1; i > 0; i--) {
    s_sample_data.samples[i] = s_sample_data.samples[i - 1];
  }

  Sample *s = &s_sample_data.samples[0];
  s->timestamp = time(NULL);
  s->charge_perc = charge_perc;
  s->last_sample_time = last_sample_time;
  s->last_charge_perc = last_charge_perc;
  s->time_diff = time_diff;
  s->charge_diff = charge_diff;
  s->result = result;
}

// Old algo for comparison
int data_calculate_avg_discharge_rate_v1() {
  SampleData *data = data_get_sample_data();
  const int count = data_get_valid_samples_count();

  // Not enough samples yet
  if (count < MIN_SAMPLES) return STATUS_EMPTY;

  int result_x2 = 0;
  int weight_x2 = 0;
  int seen = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    const int v = data->samples[i].result;
    // No discharge in this sample
    if (!util_is_valid(v)) continue;

    seen++;
    if (seen <= 4) {
      result_x2 += v * 2; // next three -> 2x weight
      weight_x2 += 2;
    } else {
      result_x2 += v * 1; // rest -> 1x weight
      weight_x2 += 1;
    }
  }

  if (weight_x2 == 0) return STATUS_EMPTY;

  return result_x2 / weight_x2;
}

/**
 * This _should_ place twice as much emphasis on the most recent half of values,
 * so recent changes in watchface or activity are more quickly reflected.
 *
 * Important: count all time periods in the log, not just those with a drop!
 */
int data_calculate_avg_discharge_rate() {
  SampleData *data = data_get_sample_data();
  const int total = data_get_log_length();

  // Not enough samples yet
  if (total < MIN_SAMPLES) return STATUS_EMPTY;

  int result_x2 = 0;
  int weight_x2 = 0;
  int count = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    int r = data->samples[i].result;

    // Log ends here, we never skip a slot when adding to it
    if (r == STATUS_EMPTY) break;
    // No charge change but we still count the time period elapsed
    if (r == STATUS_NO_CHANGE) {
      r = 0;
    }
    // Tricky - decision to ignore this time period for now (not discharging)
    if (r == STATUS_CHARGED) continue;

    count++;
    if (count <= 4) {
      result_x2 += r * 2; // next three -> 2x weight
      weight_x2 += 2;
    } else {
      result_x2 += r * 1; // rest -> 1x weight
      weight_x2 += 1;
    }
  }

  // We didn't count anything - empty log?
  if (weight_x2 == 0) return STATUS_EMPTY;

  return result_x2 / weight_x2;
}

int data_calculate_days_remaining() {
  // Use live battery level, not last reading
  const BatteryChargeState state = battery_state_service_peek();
  const int charge_perc = state.charge_percent;
  const int rate = data_calculate_avg_discharge_rate();

  // Given a 'valid' log entry is only one with a discharging change
  if (!util_is_valid(rate)) return STATUS_EMPTY;

  // We only ever charged in every non-empty log entry...
  if (rate <= 0) return STATUS_EMPTY;

  return charge_perc / rate;
}

bool data_get_rate_is_elevated() {
  // Return true if the most recent value is much higher than usual
  const int avg = data_calculate_avg_discharge_rate();
  const int last = s_sample_data.samples[0].result;

  // Last value wasn't a significant one of discharge
  if (!util_is_valid(avg) || !util_is_valid(last)) return false;

  return last >= avg * EVEVATED_RATE_MULT;
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

int data_get_valid_samples_count() {
  int count = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    if (util_is_valid(s_sample_data.samples[i].result)) {
      count++;
    }
  }
  return count;
}

int data_get_log_length() {
  int count = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    // Count all, regardless of status unless they're truly empty
    if (s_sample_data.samples[i].result != STATUS_EMPTY) {
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

bool data_get_push_timeline_pins() {
  return s_app_data.push_timeline_pins;
}

void data_set_push_timeline_pins(bool b) {
  s_app_data.push_timeline_pins = b;
}

bool data_get_elevated_rate_alert() {
  return s_app_data.elevated_rate_alert;
}

void data_set_elevated_rate_alert(bool b) {
  s_app_data.elevated_rate_alert = b;
}
