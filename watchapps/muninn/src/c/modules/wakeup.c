#include "wakeup.h"

void wakeup_unschedule() {
  wakeup_cancel_all();
  data_set_wakeup_id(DATA_EMPTY);

  APP_LOG(APP_LOG_LEVEL_INFO, "Unscheduled wakeup");
}

void wakeup_schedule_next() {
  // We only want one - this one
  wakeup_unschedule();

  // Next 12 o'clock, then every 12h after
  const time_t ts = time(NULL);
  const struct tm *now = gmtime(&ts);
  
  // Timestamp in seconds until next mid-day or mid-night, whichever is closest to now
  const int today_s = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
  const int target = (today_s < 12 * 3600) ? (12 * 3600) : (24 * 3600);
  const int interval_s = target - today_s;
  // const int interval_s = 120;  // Test short term wakeups
  const time_t future = ts + interval_s + 10;
  APP_LOG(APP_LOG_LEVEL_INFO, "Seconds until next 12h: %d", interval_s);
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

  // We popped
  data_set_wakeup_id(DATA_EMPTY);
  alert_window_push(
    RESOURCE_ID_WRITING,
    is_low ? "Muninn advises that the battery charge is below your custom threshold." : "Muninn takes a note...",
    data_get_vibe_on_sample(),
    !is_low
  );

  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;
  const time_t now = time(NULL);

  // First ever sample - nothing to compare to
  if (data_get_history_avg_rate() == DATA_EMPTY) {
    APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");

    data_set_discharge_start_time(now);
  } else {
    // Calculate change in charge
    const int charge_diff = data_get_last_charge_perc() - charge_percent;

    // Ignore if plugged in or recently charged (store discharge rates only)
    if (is_plugged || charge_diff < 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring: plugged in or recently charged");
    } else {
      // Unplugged since last sample
      if ((data_get_was_plugged() && !is_plugged)) {
        data_set_discharge_start_time(now);
      }

      // Calculate new discharge rate - assumes fixed interval
      const int perc_per_day = (charge_diff * HOURS_PER_DAY) / WAKEUP_MOD_H;
      data_push_sample_value(perc_per_day);
      APP_LOG(APP_LOG_LEVEL_INFO, "perc_per_day: %d", perc_per_day);
    }
  }

  // Store current data
  data_set_was_plugged(is_plugged);
  data_set_last_charge_perc(charge_percent);

  data_log_state();
  wakeup_schedule_next();
}
