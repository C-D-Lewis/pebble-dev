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

  // Init all fields in Sample struct
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

void data_init() {
#if defined(USE_TEST_DATA)
  // Arbitrary scenario - (reverse order)
  // const int changes[NUM_SAMPLES] = {30, 30, 30, 30, 30, 30, 30, 30};
  // Test case: Should show 10 days at 8% per day
  const int changes[NUM_SAMPLES] = {2, 2, 2, 2, 2, 2, 2, 2};
  // Test case: Should show 10 days at 8% (two other events are ignored)
  // const int changes[NUM_SAMPLES] = {2, 2, 2, 2, -20, 2, 0, 2};
  // Special status scenario
  // const int changes[NUM_SAMPLES] = {2, 2, 2, 2, 0, 2, -20, 2};

  int total_change = 0;
  for(int i = 0; i < NUM_SAMPLES; i++) {
    total_change += changes[i];
  }
  const int interval_s = WAKEUP_MOD_H * SECONDS_PER_HOUR;

  // Load test values for this launch
  // TODO: Use the array's newest value here
  s_app_data.last_sample_time = time(NULL) - interval_s;
  s_app_data.last_charge_perc = 80;
  s_app_data.wakeup_id = time(NULL) + (12 * SECONDS_PER_HOUR);   // Won't be found
  s_app_data.seen_first_launch = true;
  s_app_data.vibe_on_sample = true;
  s_app_data.custom_alert_level = AL_20;
  s_app_data.ca_has_notified = false;

  const time_t now = time(NULL);
  for (int i = NUM_SAMPLES - 1; i >= 0; i--) {
    Sample *s = &s_sample_data.samples[i];
    const int gap = changes[NUM_SAMPLES - 1 - i];

    if (i == NUM_SAMPLES - 1) {
      // Oldest sample
      s->timestamp = now - ((NUM_SAMPLES - 1) * interval_s);
      s->result = gap == 0 ? STATUS_NO_CHANGE : (gap * 4);
      s->charge_diff = changes[NUM_SAMPLES - 1];
      s->last_sample_time = now - ((NUM_SAMPLES - 2) * interval_s);
      s->time_diff = interval_s;
      s->charge_perc = 80;
      s->last_charge_perc = s->charge_perc + s->charge_diff;
      continue;
    }

    // Diff the last
    Sample *last_s = &s_sample_data.samples[i + 1];

    s->timestamp = last_s->timestamp - interval_s;
    s->charge_diff = gap;
    s->last_sample_time = s->timestamp + interval_s;
    s->time_diff = interval_s;
    s->charge_perc = last_s->charge_perc - s->charge_diff;
    s->last_charge_perc = s->charge_perc + s->charge_diff;

    // Hacky way to simulate both special statuses
    s->result = gap == 0 ? STATUS_NO_CHANGE : (gap * 4);
    if (s->charge_perc > last_s->charge_perc) {
      s->result = STATUS_CHARGED;
    }
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

void data_activation_update() {
  BatteryChargeState state = battery_state_service_peek();
  const int charge_percent = state.charge_percent;
  const time_t now = time(NULL);

  // If a fairly close amount of time to the full period is left, capture the level
  // This should help with initial under-estimation of the discharge rate
  // and get started slightly quicker
  if (util_hours_until_next_interval() >= (3 * WAKEUP_MOD_H) / 4) {
    data_set_last_sample_time(now);
    data_set_last_charge_perc(charge_percent);
  }
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

/**
 * This _should_ place twice as much emphasis on the most recent half of values,
 * so recent changes in watchface or activity are more quickly reflected.
 */
int data_calculate_avg_discharge_rate() {
  SampleData *data = data_get_sample_data();
  const int count = data_get_valid_samples_count();

  // Not enough samples yet
  if (count < MIN_SAMPLES) return STATUS_EMPTY;

#if defined(LOG_AVERAGING)
  APP_LOG(APP_LOG_LEVEL_INFO, "avg: count: %d", count);
#endif

  int result_x3 = 0;
  int weight_x3 = 0;
  int seen = 0;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    const int v = data->samples[i].result;
    // No discharge in this sample
    if (!util_is_valid(v)) continue;

    seen++;
    if (seen == 1) {
      result_x3 += v * 3; // first item -> 3x weight
      weight_x3 += 3;
#if defined(LOG_AVERAGING)
      APP_LOG(APP_LOG_LEVEL_INFO, "avg: i:%d x3 %d (seen: %d)", i, v, seen);
#endif
    } else if (seen <= 4) {
      result_x3 += v * 2; // next three -> 2x weight
      weight_x3 += 2;
#if defined(LOG_AVERAGING)
      APP_LOG(APP_LOG_LEVEL_INFO, "avg: i:%d x2 %d (seen: %d)", i, v, seen);
#endif
    } else {
      result_x3 += v * 1; // rest -> 1x weight
      weight_x3 += 1;
#if defined(LOG_AVERAGING)
      APP_LOG(APP_LOG_LEVEL_INFO, "avg: i:%d x1 %d (seen: %d)", i, v, seen);
#endif
    }
  }

  if (weight_x3 == 0) return STATUS_EMPTY;
  const int avg = result_x3 / weight_x3;
#if defined(LOG_AVERAGING)
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "avg: result_x3 %d / weight_x3 %d => %d\n---",
    result_x3,
    weight_x3,
    avg
  );
#endif
  return avg;
}

int data_calculate_days_remaining() {
  int rate = data_calculate_avg_discharge_rate();
  int charge_perc = data_get_last_charge_perc();

  if (!util_is_valid(rate) || !util_is_valid(charge_perc)) return STATUS_EMPTY;

  if (rate <= 0) {
    APP_LOG(APP_LOG_LEVEL_INFO, "zero or negative rate: %d charge_perc: %d", rate, charge_perc);
    return STATUS_EMPTY;
  }

  return charge_perc / rate;
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
