#include "wakeup.h"

// TODO: Use retval for UI
bool wakeup_schedule_next() {
  // Only once wakeup at a time
  if (data_get_wakeup_id() != DATA_EMPTY) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Wakeup already scheduled!");
    return false;
  }

  const time_t future = time(NULL) + WAKEUP_INTERVAL_S;
  const int cookie = 0;
  const int id = wakeup_schedule(future, cookie, false);

  // Check the scheduling was successful
  if (id >= 0) {
    // Verify scheduled
    if (!wakeup_query(id, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup!");
      return false;
    }

    data_set_wakeup_id(id);
    APP_LOG(APP_LOG_LEVEL_INFO, "Scheduled wakeup: %d", id);
    return true;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup: %d", id);
    return false;
  }
}

void wakeup_unschedule() {
  wakeup_cancel_all();
  data_set_wakeup_id(DATA_EMPTY);

  APP_LOG(APP_LOG_LEVEL_INFO, "Unscheduled wakeup");
}

void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  // We popped
  data_set_wakeup_id(DATA_EMPTY);
  wakeup_window_push();

  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;

  const time_t last_update_time = data_get_last_update_time();
  const time_t now = time(NULL);

  // First ever sample
  if (last_update_time == DATA_EMPTY) {
    APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");

    data_set_discharge_start_time(now);
  } else {
    // Calculate change in charge
    const int discharge_diff = data_get_last_charge_perc() - charge_percent;

    // Calculate time since last sample
    time_t diff_s = now - last_update_time;
    if (diff_s == 0) diff_s = 1;
    APP_LOG(APP_LOG_LEVEL_INFO, "diff_s: %ds, discharge_diff: %d%%", (int)diff_s, discharge_diff);

    if (is_plugged || discharge_diff < 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring, plugged in or recently charged");
    } else {
      if ((data_get_was_plugged() && !is_plugged)) {
        data_set_discharge_start_time(now);
      }

      // Calculate new discharge rate
      // This is done every 12 hours according to WAKEUP_INTERVAL_S
      // Therefore we can derive the daily rate based on the interval in seconds
      const int perc_per_day = (discharge_diff * SECONDS_PER_DAY) / WAKEUP_INTERVAL_S;
      data_push_sample_value(perc_per_day);
      APP_LOG(APP_LOG_LEVEL_INFO, "perc_per_day: %d", perc_per_day);
    }
  }

  // Store current data
  data_set_last_update_time(now);
  data_set_was_plugged(is_plugged);
  data_set_last_charge_perc(charge_percent);

  data_log_state();

  // Re-schedule next sample
  wakeup_schedule_next();
}
