#include "wakeup.h"

void wakeup_schedule_next() {
  // We only want one - this one
  wakeup_cancel_all();
  data_set_wakeup_id(DATA_EMPTY);

  // Next 12 o'clock, then every 12h after
  const time_t temp = time(NULL);
  const struct tm *now = gmtime(&temp);
  
  // Timestamp in seconds until next mid-day or mid-night, whichever is closest to now
  const int seconds_today = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
  const int target = (seconds_today < 12 * 3600) ? (12 * 3600) : (24 * 3600);
  const int interval_s = target - seconds_today;
  // const int interval_s = 120;  // Test short term wakeups
  const time_t future = temp + interval_s + 10;
  APP_LOG(APP_LOG_LEVEL_INFO, "Seconds until next 12h: %d", interval_s);
  const int id = wakeup_schedule(future, 0, false);

  // Check the scheduling was successful
  if (id >= 0) {
    // Verify scheduled
    if (!wakeup_query(id, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Scheduled wakeup not found");
      data_set_error("Scheduled wakeup not found");
      return;
    }

    data_set_wakeup_id(id);
    APP_LOG(APP_LOG_LEVEL_INFO, "Scheduled wakeup: %d", id);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup: %d", id);
    data_set_error("Failed to schedule wakeup");
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
  alert_window_push(
    RESOURCE_ID_WRITING,
    "Muninn takes a note...",
    true,
    true
  );

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
      // Therefore we can derive the daily rate based on the interval in seconds
      // TODO: Calculation to next 12h is not directly tied to WAKEUP_H
      const int perc_per_day = (discharge_diff * HOURS_PER_DAY) / WAKEUP_H;
      data_push_sample_value(perc_per_day);
      APP_LOG(APP_LOG_LEVEL_INFO, "perc_per_day: %d", perc_per_day);
    }
  }

  // Store current data
  data_set_last_update_time(now);
  data_set_was_plugged(is_plugged);
  data_set_last_charge_perc(charge_percent);

  data_log_state();

  wakeup_schedule_next();
}
