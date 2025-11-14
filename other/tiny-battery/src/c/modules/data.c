#include "data.h"

static int s_discharge_start_time;
static int s_last_update_time;
static int s_last_charge_perc;
static int s_wakeup_id;
static bool s_was_plugged;
static SampleData s_sample_data;

void save_all() {
  persist_write_int(SK_DischargeStartTime, s_discharge_start_time);
  persist_write_int(SK_LastUpdateTime, s_last_update_time);
  persist_write_int(SK_LastChargePerc, s_last_charge_perc);
  persist_write_int(SK_WakeupId, s_wakeup_id);
  persist_write_bool(SK_WasPlugged, s_was_plugged);

  status_t result = persist_write_data(SK_SampleData, &s_sample_data, sizeof(SampleData));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing sample data: %d", (int)result);
  }
  // TODO: Handle error in UI
}

static void delete_all_data() {
  for (int i = 0; i < SK_Max; i += 1) persist_delete(i);
  wakeup_cancel_all();
  APP_LOG(APP_LOG_LEVEL_INFO, "!!! RESET ALL DATA !!!");
}

void data_prepare() {
  // Data that should be reset when re-enabling
  s_discharge_start_time = DATA_EMPTY;
  s_last_update_time = DATA_EMPTY;
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

void data_init() {
#if defined(WIPE_ON_LAUNCH)
  delete_all_data();
#endif

  if (!persist_exists(SK_WakeupId)) {
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
      return;
    }
    // TODO Handle error in UI

    data_log_state();
  }
}

void data_deinit() {
  save_all();
}

int data_get_discharge_start_time() {
  return s_discharge_start_time;
}

void data_set_discharge_start_time(int time) {
  s_discharge_start_time = time;
}

int data_get_last_update_time() {
  return s_last_update_time;
}

void data_set_last_update_time(int time) {
  s_last_update_time = time;
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

void data_push_sample_value(int v) {
  // Shift right
  for (int i = NUM_STORED_SAMPLES - 1; i > 0; i--) {
    s_sample_data.history[i] = s_sample_data.history[i - 1];
  }
  s_sample_data.history[0] = v;
}

int data_get_history_avg_rate() {
  int acc = 0;
  int counted = 0;
  for (int i = 0; i < NUM_STORED_SAMPLES; i++) {
    if (s_sample_data.history[i] == DATA_EMPTY) continue;
    acc += s_sample_data.history[i];
    counted++;
  }
  return acc / counted;
}

void data_sample_now() {
  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;

  const time_t now = time(NULL);

  data_set_was_plugged(is_plugged);
  data_set_last_charge_perc(charge_percent);

  data_set_discharge_start_time(now);
  data_set_last_update_time(now);
}
