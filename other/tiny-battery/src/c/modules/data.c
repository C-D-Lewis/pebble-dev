#include "data.h"

static int s_discharge_start_time;
static int s_last_update_time;
static int s_last_charge_perc;
static int s_discharge_rate;
static int s_wakeup_id;
static bool s_was_plugged;

void save_all() {
  persist_write_int(SK_DischargeStartTime, s_discharge_start_time);
  persist_write_int(SK_LastUpdateTime, s_last_update_time);
  persist_write_int(SK_LastChargePerc, s_last_charge_perc);
  persist_write_int(SK_DischargeRate, s_discharge_rate);
  persist_write_int(SK_WakeupId, s_wakeup_id);
  persist_write_bool(SK_WasPlugged, s_was_plugged);

  APP_LOG(APP_LOG_LEVEL_INFO, "Saved all storage");
}

static void delete_all_data() {
  for (int i = 0; i < SK_Max; i += 1) {
    persist_delete(i);
  }
  wakeup_cancel_all();
  APP_LOG(APP_LOG_LEVEL_INFO, "!!! RESET ALL DATA !!!");
}

void data_prepare() {
  // Data that should be reset when re-enabling
  s_discharge_start_time = NO_DATA;
  s_last_update_time = NO_DATA;

  BatteryChargeState state = battery_state_service_peek();
  s_last_charge_perc = state.charge_percent;
  s_was_plugged = state.is_plugged;
}

void data_log_state() {
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "S: %d, U: %d, W: %d, P: %d, R: %d, C: %s",
    s_discharge_start_time, s_last_update_time, s_wakeup_id, s_last_charge_perc,
    s_discharge_rate, s_was_plugged ? "true": "false"
  );
}

void data_init() {
  // HACK to wipe during testing
  // delete_all_data();

  if (!persist_exists(SK_WakeupId)) {
    // Write defaults - some will be init'd when tracking begins
    s_discharge_start_time = NO_DATA;
    s_last_update_time = NO_DATA;
    s_last_charge_perc = NO_DATA;
    s_discharge_rate = NO_DATA;
    s_wakeup_id = NO_DATA;
    s_was_plugged = true;

    save_all();
  } else {
    // Load current data for foreground display
    s_discharge_start_time = persist_read_int(SK_DischargeStartTime);
    s_last_update_time = persist_read_int(SK_LastUpdateTime);
    s_last_charge_perc = persist_read_int(SK_LastChargePerc);
    s_discharge_rate = persist_read_int(SK_DischargeRate);
    s_wakeup_id = persist_read_int(SK_WakeupId);
    s_was_plugged = persist_read_bool(SK_WasPlugged);

    data_log_state();
  }
}

void data_deinit() {
  save_all();
}

int data_get_discharge_start_time(void) {
  return s_discharge_start_time;
}

void data_set_discharge_start_time(int time) {
  s_discharge_start_time = time;
}

int data_get_last_update_time(void) {
  return s_last_update_time;
}

void data_set_last_update_time(int time) {
  s_last_update_time = time;
}

int data_get_last_charge_perc(void) {
  return s_last_charge_perc;
}

void data_set_last_charge_perc(int perc) {
  s_last_charge_perc = perc;
}

int data_get_discharge_rate(void) {
  return s_discharge_rate;
}

void data_set_discharge_rate(int rate) {
  s_discharge_rate = rate;
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
