#include "data.h"

// Persisted
static PersistData s_persist_data;
static Sample s_samples[NUM_SAMPLES];

// Not persisted
static AppState s_app_state;
static char s_error_buff[64];

static void delete_all_data() {
  for (int i = 0; i < SK_Max; i += 1) persist_delete(i);
  wakeup_cancel_all();
// APP_LOG(APP_LOG_LEVEL_INFO, "!!! RESET ALL DATA !!!");
}

static void save_all() {
  status_t result = persist_write_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "data w err %d", (int)result);
    data_set_error("Error writing app data");
  }
  if (result < (int)sizeof(PersistData)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "data w trunc");
  }

  // Save each Sample using SK_SampleBase + i
  for (int i = 0; i < NUM_SAMPLES; i++) {
    const int key = SK_SampleBase + i;
    result = persist_write_data(key, &s_samples[i], sizeof(Sample));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "sample w err %d", (int)result);
      data_set_error("Error writing sample data");
    }
  }
}

static void init_data_fields() {
  s_persist_data.last_sample_time = STATUS_EMPTY;
  s_persist_data.last_charge_perc = STATUS_EMPTY;
  s_persist_data.wakeup_id = STATUS_EMPTY;
  s_persist_data.seen_first_launch = false;
  s_persist_data.vibe_on_sample = false;
  s_persist_data.custom_alert_level = AL_OFF;
  s_persist_data.ca_has_notified = false;
  s_persist_data.push_timeline_pins = false;
  s_persist_data.elevated_rate_alert = false;
  s_persist_data.one_day_notified = false;
  s_persist_data.last_charge_time = STATUS_EMPTY;
  s_persist_data.one_day_alert = false;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_samples[i];
    s->timestamp = STATUS_EMPTY;
    s->charge_perc = STATUS_EMPTY;
    s->last_sample_time = STATUS_EMPTY;
    s->last_charge_perc = STATUS_EMPTY;
    s->time_diff = STATUS_EMPTY;
    s->charge_diff = STATUS_EMPTY;
    s->result = STATUS_EMPTY;
    s->days_remaining = STATUS_EMPTY;
    s->rate = STATUS_EMPTY;
  }
}

void data_reset_all() {
  delete_all_data();
  init_data_fields();
  save_all();
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

static void test_data_generator() {
  //
  // Test data scenarios
  //
  // 1 - Arbitrary scenario
  const int changes[NUM_SAMPLES] = {0, 2, 2, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  //
  // 2 - Test case: Should show 10 days at 8% per day (from 80%)
  // const int changes[NUM_SAMPLES] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  //
  // 3 - Test case: Should show 11 days at 7% (two other events are ignored)
  //     Note: includes the two special statuses
  // const int changes[NUM_SAMPLES] = {-20, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
  //
  // 4 - Test case: Should show 6 days at 12% per day (from 80%)
  // const int changes[NUM_SAMPLES] = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
  //
  // 5 - Test case: Should await 2 samples if both taken are 'no change'
  // const int changes[NUM_SAMPLES] = {0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  //            or: Should allow one discharge and one 'no change'
  // const int changes[NUM_SAMPLES] = {2, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  //            or: Should await 2 samples if charged and 'no change'
  // const int changes[NUM_SAMPLES] = {0, -10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  //            or: Should await 1 sample
  // const int changes[NUM_SAMPLES] = {2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  //
  // 6 - Test case: Should show 1% when the majority of events are 'no change' (extreme battery life)
  // const int changes[NUM_SAMPLES] = {0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0};
  //
  // 7 - Test case: Big charge half way through
  // const int changes[NUM_SAMPLES] = {3, 0, 3, 3, 0, 3, 3, 3, -30, 3, 3, 3, 3, 3, 3, 3};
  //
  // 8 - Test case: Should show graph with minimum points
  // const int changes[NUM_SAMPLES] = {3, 1, 2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  const int interval_s = WAKEUP_MOD_H * SECONDS_PER_HOUR;

  // Use the most recent interval time as a base with 0 minutes
  const time_t now = time(NULL);
  struct tm *ts_info = localtime(&now);
  ts_info->tm_hour -= (ts_info->tm_hour % WAKEUP_MOD_H);
  ts_info->tm_min = 0;
  ts_info->tm_sec = 5;
  const time_t base = mktime(ts_info);

  // Set fixed test values for this launch
  // TODO: Use the array's newest value here?
  s_persist_data.last_sample_time = base;
  s_persist_data.last_charge_perc = 80;  // Agrees with emulator
  s_persist_data.wakeup_id = base + (12 * SECONDS_PER_HOUR);  // Won't be found
  s_persist_data.seen_first_launch = true;
  s_persist_data.vibe_on_sample = true;
  s_persist_data.custom_alert_level = AL_20;
  s_persist_data.ca_has_notified = false;
  s_persist_data.elevated_rate_alert = false;
  s_persist_data.push_timeline_pins = false;
  s_persist_data.one_day_notified = false;
  s_persist_data.last_charge_time = base - (3 * SECONDS_PER_DAY);
  s_persist_data.one_day_alert = false;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_samples[i];
    const int gap = changes[i];
    if (gap == STATUS_EMPTY) continue;

    if (i == 0) {
      s->timestamp = base;
      s->charge_perc = s_persist_data.last_charge_perc;
      s->last_sample_time = base - interval_s;
      s->last_charge_perc = s->charge_perc + gap;
      s->charge_diff = gap;
      s->time_diff = interval_s;
      s->result = result_from_gap(gap);

      if (util_is_not_status(s->result)) {
        // TODO: Generator should use accumulated average rate, not one sample's
        s->days_remaining = s->charge_perc / s->result;
        s->rate = s->result;
      }
      continue;
    }

    // Use the previous in the array which is more recent
    Sample *newer_s = &s_samples[i - 1];

    s->timestamp = newer_s->last_sample_time;
    s->charge_perc = newer_s->last_charge_perc;
    s->last_sample_time = s->timestamp - interval_s;
    s->last_charge_perc = s->charge_perc + gap;
    s->charge_diff = gap;
    s->time_diff = interval_s;
    s->result = result_from_gap(gap);
    // Make this deliberately incorrect for testing
    if (util_is_not_status(s->result)) {
      const int result = s->result;
      s->days_remaining = s->charge_perc / result;
      s->rate = result;
    }
  }
}
#endif

// Handle new fields with default values
static void handle_new_fields() {
  if (s_persist_data.last_charge_time == 0) {
    s_persist_data.last_charge_time = STATUS_EMPTY;
  }
}

void data_init() {
  // Before anything else, check if we should reset data
  if (!persist_exists(SK_Migration_1)) {
    data_reset_all();
    persist_write_int(SK_Migration_1, 1);
  }

#if defined(USE_TEST_DATA)
  init_data_fields();
  test_data_generator();
  data_log_state();
  return;
#endif

  // Init AppState emphemeral fields
  s_app_state.sync_count = STATUS_EMPTY;

  // Never used, write defaults
  if (!persist_exists(SK_PersistData)) {
    data_reset_all();
  } else {
    // Load current data
    status_t result = persist_read_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "app read fail %d", (int)result);
      data_set_error("Error reading app data");
      return;
    }

    // Load each sample using SK_SampleBase + i
    for (int i = 0; i < NUM_SAMPLES; i++) {
      const int key = SK_SampleBase + i;
      result = persist_read_data(key, &s_samples[i], sizeof(Sample));
      if (result < 0) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "sample read fail %d", (int)result);
        data_set_error("Error reading sample data");
        return;
      }
    }
  }

  handle_new_fields();
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
  const bool is_enabled = util_is_not_status(s_persist_data.wakeup_id);
  if (is_enabled) {
    wakeup_query(s_persist_data.wakeup_id, &wakeup_ts);
  }

  // App state
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "lst:%d lcp:%d wi:%d wts:%d sfl:%s vos:%s cal:%d chn:%s",
    s_persist_data.last_sample_time,
    s_persist_data.last_charge_perc,
    s_persist_data.wakeup_id,
    (int)wakeup_ts,
    s_persist_data.seen_first_launch == 1 ? "T" : "F",
    s_persist_data.vibe_on_sample == 1 ? "T" : "F",
    s_persist_data.custom_alert_level,
    s_persist_data.ca_has_notified == 1 ? "T" : "F"
  );

  // Sample history
  APP_LOG(APP_LOG_LEVEL_INFO, "i,lst,ts,td,lcp,cp,cd,r,dr,rt");
  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_samples[i];
    APP_LOG(
      APP_LOG_LEVEL_INFO,
      "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
      i,
      (int)s->last_sample_time,
      (int)s->timestamp,
      (int)s->time_diff,
      (int)s->last_charge_perc,
      (int)s->charge_perc,
      (int)s->charge_diff,
      (int)s->result,
      (int)s->days_remaining,
      (int)s->rate
    );
  }
#endif
}

void data_push_sample(int charge_perc, int last_sample_time, int last_charge_perc, int time_diff, int charge_diff, int result) {
  // Shift right previous samples
  for (int i = NUM_SAMPLES - 1; i > 0; i--) {
    s_samples[i] = s_samples[i - 1];
  }

  Sample *s = &s_samples[0];
  s->timestamp = time(NULL);
  s->charge_perc = charge_perc;
  s->last_sample_time = last_sample_time;
  s->last_charge_perc = last_charge_perc;
  s->time_diff = time_diff;
  s->charge_diff = charge_diff;
  s->result = result;
  if (util_is_not_status(result)) {
    s->days_remaining = data_calculate_days_remaining();
    s->rate = data_calculate_avg_discharge_rate();
  } else {
    s->days_remaining = STATUS_EMPTY;
    s->rate = STATUS_EMPTY;
  }
}

/**
 * This _should_ place twice as much emphasis on the most recent four values,
 * so recent changes in watchface or activity are more quickly reflected.
 *
 * Important: count all time periods in the log, not just those with a drop!
 */
int data_calculate_avg_discharge_rate() {
  const int total = data_get_valid_samples_count();
  if (total < MIN_SAMPLES) return STATUS_EMPTY;

  int total_x2 = 0;
  int weight_x2 = 0;
  int count = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    const Sample *s = &s_samples[i];
    int result = s->result;

    // Log ends here, we never skip a slot when adding to it
    if (result == STATUS_EMPTY) break;
    // No charge change but we still count the time period elapsed
    if (result == STATUS_NO_CHANGE) {
      result = 0;
    }
    // Tricky - decision to ignore this time period for now (not discharging time)
    if (result == STATUS_CHARGED) continue;

    count++;

    // Weight the most recent 4 samples (last day) more heavily
    if (count <= 4) {
      total_x2 += result * 2;
      weight_x2 += 2;
    } else {
      total_x2 += result * 1;
      weight_x2 += 1;
    }
  }

  // We didn't count anything - empty log?
  if (weight_x2 == 0) return STATUS_EMPTY;

  // If the majority samples are no-change, we may have a zero rate average
  // This case is a problem: 90% charge becomes 90 days at 1% rate...
  if (total_x2 == 0) return 1;

  return total_x2 / weight_x2;
}

int data_calculate_days_remaining() {
  // Use live battery level, not last reading
  const BatteryChargeState state = battery_state_service_peek();
  const int charge_perc = state.charge_percent;
  const int rate = data_calculate_avg_discharge_rate();

  // If not enough data
  if (data_get_valid_samples_count() < MIN_SAMPLES) return STATUS_EMPTY;
  // Data not available yet
  if (!util_is_not_status(rate)) return STATUS_EMPTY;
  // Only ever charged, or rate is zero ('return 1' above should prevent this)
  if (rate <= 0) return STATUS_EMPTY;

  return charge_perc / rate;
}

bool data_get_rate_is_elevated() {
  // Return true if the most recent value is much higher than usual
  const int avg = data_calculate_avg_discharge_rate();
  const int last = s_samples[0].result;

  // Last value wasn't a significant one of discharge
  if (!util_is_not_status(avg) || !util_is_not_status(last)) return false;

  return last >= avg * EVEVATED_RATE_MULT;
}

void data_cycle_custom_alert_level() {
  switch (s_persist_data.custom_alert_level) {
    case AL_OFF:
      s_persist_data.custom_alert_level = AL_50;
      break;
    case AL_50:
      s_persist_data.custom_alert_level = AL_20;
      break;
    case AL_20:
      s_persist_data.custom_alert_level = AL_10;
      break;
    case AL_10:
    default:
      s_persist_data.custom_alert_level = AL_OFF;
      break;
  }

  // If changed to less than current, re-enable notification
  s_persist_data.ca_has_notified = false;
}

time_t data_get_next_charge_time() {
  const time_t now = time(NULL);
  const int days_remaining = data_calculate_days_remaining();
  if (!util_is_not_status(days_remaining)) return STATUS_EMPTY;

  return now + (days_remaining * SECONDS_PER_DAY);
}

int data_calculate_accuracy() {
  int newest_i = 0;
  int oldest_i = -1;

  // Find oldest valid sample
  for (int i = 0; i < NUM_SAMPLES; i++) {
    if (s_samples[i].result == STATUS_EMPTY) break;
    oldest_i = i;
  }
  // Not enough samples
  if (oldest_i <= newest_i) return STATUS_EMPTY;

  int expected_acc = 0;
  int actual_acc = 0;

  // Bodge: pre-fill last_valid_rate in case the first rate ever is missing
  int last_valid_rate = 0;
  for (int i = newest_i; i <= oldest_i; i++) {
    if (util_is_not_status(s_samples[i].rate)) {
      last_valid_rate = s_samples[i].rate;
      break;
    }
  }

  for (int i = newest_i; i < oldest_i; i++) {
    Sample *current = &s_samples[i];
    Sample *older = &s_samples[i + 1];

    if (util_is_not_charging(current->result)) {
      // Accumulate actual change
      if (older->charge_perc >= current->charge_perc) {
        actual_acc += (older->charge_perc - current->charge_perc);
      }

      // If current rate is valid, use it and update fallback
      if (util_is_not_status(current->rate)) {
        last_valid_rate = current->rate;
        expected_acc += current->rate * current->time_diff;
      } else if (last_valid_rate > 0) {
        // If current rate is invalid, use the last known good rate
        expected_acc += last_valid_rate * current->time_diff;
      }
    }
  }

  // Prevent divide by zero - somehow nothing dropped
  if (expected_acc <= 0 || actual_acc <= 0) return STATUS_EMPTY;

  // Convert expected_acc (percent-seconds) to raw percentage points
  const int exp_total_perc = expected_acc / SECONDS_PER_DAY;

  // Return the difference in battery gauge units
  return actual_acc - exp_total_perc;
}

// Currently not used
int data_calculate_days_remaining_accuracy() {
  // If there is at least a whole day of continuous discharge samples, compare the
  // days_remaining estimate at the start and end of that period.
  int start_i = -1;
  int end_i = -1;
  int time_acc = 0;
  bool discharged = false;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    Sample *s = &s_samples[i];
    // Log ends here / no data
    if (s->result == STATUS_EMPTY) break;

    // Only consider discharging or 'no change' samples
    if (util_is_not_charging(s->result)) {
      if (start_i == -1) {
        start_i = i;
      }
      end_i = i;
      time_acc += s->time_diff;

      // Must be at least one discharging sample in the period
      if (util_is_not_status(s->result)) {
        discharged = true;
      }
    } else {
      // Reset if we hit a non-discharging sample
      start_i = -1;
      end_i = -1;
      time_acc = 0;
      discharged = false;
    }
  }

  // No valid full day or more period found including some discharge
  if (
    start_i == -1 || end_i == -1 ||
    start_i == end_i ||
    time_acc < SECONDS_PER_DAY ||
    !discharged) {
    return STATUS_EMPTY;
  }

  Sample *newest_s = &s_samples[start_i];
  Sample *oldest_s = &s_samples[end_i];

  // Somehow invalid days_remaining values?
  if (
    !util_is_not_status(newest_s->days_remaining) ||
    !util_is_not_status(oldest_s->days_remaining)
  ) return STATUS_EMPTY;

  // Physical time elapsed
  const int actual_elapsed = time_acc / SECONDS_PER_DAY;
  // Change in battery estimate
  const int estimated_elapsed = oldest_s->days_remaining - newest_s->days_remaining;

  if (actual_elapsed <= 0) return STATUS_EMPTY;

  // APP_LOG(
  //   APP_LOG_LEVEL_INFO,
  //   "ActualElapsed:%d EstElapsed:%d",
  //   actual_elapsed,
  //   estimated_elapsed
  // );

  return actual_elapsed - estimated_elapsed;
}

int data_get_valid_samples_count() {
  int count = 0;
  bool discharged = false;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    // Count only discharging and no-change samples
    const int r = s_samples[i].result;
    if (r != STATUS_EMPTY && r != STATUS_CHARGED) {
      count++;
    }
    if (util_is_not_status(r)) {
      discharged = true;
    }
  }

  // At least one must a be a full discharging sample
  // This means old watches with a good battery may wait longer for stats to appear
  if (!discharged) return 0;

  return count;
}

int data_get_log_length() {
  int count = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    // Count all, regardless of status unless they're truly empty
    if (s_samples[i].result != STATUS_EMPTY) {
      count++;
    }
  }
  return count;
}

///////////////////////////////////////// Getters / Setters ////////////////////////////////////////

void data_set_error(char *err) {
  snprintf(s_error_buff, sizeof(s_error_buff), "Error: %s", err);
  message_window_push(data_get_error(), true, false);
}

char* data_get_error() {
  return &s_error_buff[0];
}

Sample* data_get_sample(int index) {
  return &s_samples[index];
}

PersistData* data_get_persist_data() {
  return &s_persist_data;
}

AppState* data_get_app_state() {
  return &s_app_state;
}
