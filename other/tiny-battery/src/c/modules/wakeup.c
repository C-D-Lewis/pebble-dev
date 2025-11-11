#include "wakeup.h"

void wakeup_schedule_next() {
  // Only once wakeup at a time
  if (data_get_wakeup_id() != NO_DATA) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Wakeup already scheduled!");
    return;
  }

  const time_t future = time(NULL) + WAKEUP_INTERVAL_S;
  const int cookie = 0;
  const int id = wakeup_schedule(future, cookie, false);

  // Check the scheduling was successful
  if (id >= 0) {
    // Verify scheduled
    if (!wakeup_query(id, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup!");
      return;
    }

    data_set_wakeup_id(id);
    APP_LOG(APP_LOG_LEVEL_INFO, "Scheduled wakeup: %d", id);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup: %d", id);
  }
}

void wakeup_unschedule() {
  wakeup_cancel_all();
  data_set_wakeup_id(WAKEUP_NO_WAKEUP);

  APP_LOG(APP_LOG_LEVEL_INFO, "Unscheduled wakeup");
}

// Wait for 100% and charging before logging?

void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  // We popped
  data_set_wakeup_id(NO_DATA);

  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;

  const time_t last_update_time = data_get_last_update_time();
  const time_t now = time(NULL);

  // First ever sample
  if (last_update_time == NO_DATA) {
    APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");

    data_set_discharge_start_time(now);
  } else {
    if (data_get_was_plugged() && !is_plugged) {
      APP_LOG(APP_LOG_LEVEL_INFO, "We are now discharging");

      data_set_discharge_start_time(now);
    } else if (is_plugged) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring, plugged in");
    } else {
      // Calculate time since last sample
      const time_t diff = now - last_update_time;
      APP_LOG(APP_LOG_LEVEL_INFO, "Time diff: %ds", (int)diff);

      // Calculate change in charge
      const int charge_diff = data_get_last_charge_perc() - charge_percent;
      APP_LOG(APP_LOG_LEVEL_INFO, "Charge diff: %d", charge_diff);

      // Calculate new discharge rate (inc. old value)
      // % per hour = start_perc - end_perc / hour_diff
    }
  }

  // Store current data
  data_set_last_update_time(now);
  data_set_was_plugged(is_plugged);
  data_set_last_charge_perc(charge_percent);

  // Re-schedule next sample
  wakeup_schedule_next();

  // Show UI
  data_log_state();
  wakeup_window_push();
}
