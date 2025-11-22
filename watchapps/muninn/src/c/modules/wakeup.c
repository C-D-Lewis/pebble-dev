#include "wakeup.h"

void wakeup_unschedule() {
  wakeup_cancel_all();
  data_set_wakeup_id(DATA_EMPTY);
}

void wakeup_schedule_next() {
  // We only want one - this one
  wakeup_unschedule();

  const time_t ts_now = time(NULL);
  const struct tm *now = gmtime(&ts_now);

  // Next interval hour based on WAKEUP_MOD_H
  const int elap_min_s = now->tm_min * SECONDS_PER_MINUTE;
  const int hours_rem = WAKEUP_MOD_H - (now->tm_hour % WAKEUP_MOD_H);
  const int interval_s = (hours_rem * SECONDS_PER_HOUR) - elap_min_s - now->tm_sec;

  const time_t future = ts_now + interval_s + 5;
  // const time_t future = ts_now + 60; // For testing only
  APP_LOG(APP_LOG_LEVEL_INFO, "Seconds until next interval: %d", interval_s);
  const int id = wakeup_schedule(future, 0, false);

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

void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  // Should we advise battery is low?
  const int alert_level = data_get_custom_alert_level();
  const bool is_low = alert_level != AL_OFF && data_get_last_charge_perc() <= alert_level;
  const bool ca_has_notified = data_get_ca_has_notified();

  if (is_low && !ca_has_notified) {
    data_set_ca_has_notified(true);

    alert_window_push(
      RESOURCE_ID_WRITING,
      "Muninn advises the battery is below your custom threshold.",
      true,
      false
    );
  } else {
    if (!is_low && ca_has_notified) {
      // Reset notification flag when above threshold
      data_set_ca_has_notified(false);
    }

    // Regular wakeup window
    alert_window_push(
      RESOURCE_ID_WRITING,
      "Muninn takes a note...",
      data_get_vibe_on_sample(),
      true
    );
  }

  // We popped
  data_set_wakeup_id(DATA_EMPTY);

  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;
  const time_t now = time(NULL);

  const int last_sample_time = data_get_last_sample_time();

  // First ever sample - nothing to compare to
  if (last_sample_time == DATA_EMPTY) {
    APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");
  } else {
    const int time_diff_s = now - last_sample_time;
    const int charge_diff = data_get_last_charge_perc() - charge_percent;
    APP_LOG(APP_LOG_LEVEL_INFO, "Time diff: %d, Charge diff: %d", time_diff_s, charge_diff);

    // Ignore if plugged in or recently charged (store discharge rates only)
    if (is_plugged || charge_diff < 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring: plugged in or recently charged");

      // Maintain charge level if it's going up
      data_set_last_charge_perc(charge_percent);
    } else if (charge_diff == 0) {
      // No change since last sample
      APP_LOG(APP_LOG_LEVEL_INFO, "No change since last sample");
    } else {
      // Unplugged since last sample
      if ((data_get_was_plugged() && !is_plugged)) {
        // data_set_discharge_start_time(now);
      }

      // Calculate new discharge rate
      const int perc_per_day = (charge_diff * SECONDS_PER_DAY) / time_diff_s;
      data_push_sample_value(perc_per_day);
      APP_LOG(APP_LOG_LEVEL_INFO, "perc_per_day: %d", perc_per_day);

      // Remember these for next sample itself, not next wakeup
      data_set_last_charge_perc(charge_percent);
      data_set_last_sample_time(now);
    }
  }

  data_set_was_plugged(is_plugged);

  data_log_state();
  wakeup_schedule_next();
}
