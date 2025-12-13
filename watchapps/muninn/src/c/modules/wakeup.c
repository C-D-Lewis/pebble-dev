#include "wakeup.h"

void wakeup_unschedule() {
  wakeup_cancel_all();
  data_set_wakeup_id(STATUS_EMPTY);
}

void wakeup_schedule_next() {
  // We only want one - this one
  wakeup_unschedule();

  const time_t ts_now = time(NULL);
  const struct tm *now = localtime(&ts_now);

  // Find next interval hour based on WAKEUP_MOD_H
  const int elap_min_s = now->tm_min * 60;
  const int hours_rem = WAKEUP_MOD_H - (now->tm_hour % WAKEUP_MOD_H);
  const int interval_s = (hours_rem * SECONDS_PER_HOUR) - elap_min_s - now->tm_sec;

#if defined(WAKEUP_NEXT_MINUTE)
  // For faster testing of wakeups
  const time_t future = ts_now + 60;
#else
  // Tiny extra offset in case weird things happen exactly on the hour
  const time_t future = ts_now + interval_s + 5;
#endif

  APP_LOG(APP_LOG_LEVEL_INFO, "Seconds until next interval: %d", interval_s);
  int id = wakeup_schedule(future, 0, true);
#if defined(TEST_COLLISION)
  // To test collision rescheduling
  id = wakeup_schedule(future, 0, true);
#endif

  // Failed for some reason
  if (id < 0) {
    if (id == E_RANGE) {
      // Try again in the future if a collision with another app
      int extra_mins = 1;
      while (id < 0 && extra_mins <= EXTRA_MINUTES_MAX) {
        APP_LOG(APP_LOG_LEVEL_INFO, "E_RANGE, trying again with +%dm", extra_mins);
        id = wakeup_schedule(future + (extra_mins * 60), 0, true);
        extra_mins++;
      }
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup: %d", id);
      data_set_error("Failed to schedule wakeup");
      return;
    }
  }

  // Verify scheduled - not observed but Dashboard checks for some reason
  if (!wakeup_query(id, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Scheduled wakeup not found");
    data_set_error("Scheduled wakeup not found");
    return;
  }

  data_set_wakeup_id(id);
  APP_LOG(APP_LOG_LEVEL_INFO, "Scheduled wakeup: %d", id);
}

void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  // TODO: Can we just use the last sample?
  const int last_charge_perc = data_get_last_charge_perc();

  // We popped
  data_set_wakeup_id(STATUS_EMPTY);

  BatteryChargeState state = battery_state_service_peek();
  const bool is_plugged = state.is_plugged;
  const int charge_percent = state.charge_percent;
  const time_t now = time(NULL);
  const int last_sample_time = data_get_last_sample_time();

  // First ever sample - nothing to compare to
  if (!util_is_valid(last_sample_time)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");

    // Record state and wait for next time
    data_set_last_charge_perc(charge_percent);
    data_set_last_sample_time(now);
  } else {
    const int time_diff_s = now - last_sample_time;
    const int charge_diff = last_charge_perc - charge_percent;
    APP_LOG(APP_LOG_LEVEL_INFO, "Time diff: %d, Charge diff: %d", time_diff_s, charge_diff);

    // Turns out, in all cases below we note both of these anyway
    data_set_last_charge_perc(charge_percent);
    data_set_last_sample_time(now);

    // Ignore if plugged in or recently charged (store discharge rates only)
    if (is_plugged || charge_diff < 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Ignoring: plugged in or recently charged");

      // Record special status sample
      data_push_sample(charge_percent, last_sample_time, last_charge_perc, time_diff_s, charge_diff, STATUS_CHARGED);
    } else if (charge_diff == 0) {
      // No change since last sample - probably on charge or very short period
      APP_LOG(APP_LOG_LEVEL_INFO, "No change since last sample");

      // Record special status sample
      data_push_sample(charge_percent, last_sample_time, last_charge_perc, time_diff_s, charge_diff, STATUS_NO_CHANGE);
    } else {
      // Calculate new discharge rate
      const int result = (charge_diff * SECONDS_PER_DAY) / time_diff_s;

      data_push_sample(charge_percent, last_sample_time, last_charge_perc, time_diff_s, charge_diff, result);
      APP_LOG(APP_LOG_LEVEL_INFO, "result: %d", result);
    }
  }

  data_log_state();
  wakeup_schedule_next();

  // Should we advise battery is low?
  const int alert_level = data_get_custom_alert_level();
  const bool is_low = alert_level != AL_OFF && last_charge_perc <= alert_level;
  const bool ca_has_notified = data_get_ca_has_notified();

  // If we have data, and it's lower, and we haven't notified
  if (data_get_valid_samples_count() > MIN_SAMPLES && is_low && !ca_has_notified) {
    data_set_ca_has_notified(true);

    alert_window_push(
      RESOURCE_ID_WRITING,
      "Muninn advises the battery is below your custom threshold.",
      true,
      false
    );
    return;
  }
  if (!is_low && ca_has_notified) data_set_ca_has_notified(false);

  // Just a sample
  alert_window_push(
    RESOURCE_ID_WRITING,
    "Muninn is taking a note...",
    data_get_vibe_on_sample(),
    true
  );
}

bool wakeup_handle_missed() {
  const int wakeup_id = data_get_wakeup_id();
  if (!util_is_valid(wakeup_id)) return false;

  time_t wakeup_ts;
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);

  // Doesn't exist or is too long ago, reschedule it
  if (!found || (time(NULL) - wakeup_ts) > (WAKEUP_MOD_H * SECONDS_PER_HOUR)) {
    APP_LOG(
      APP_LOG_LEVEL_INFO,
      "Missed wakeup detected: %d %d %d",
      wakeup_id,
      found ? 1 : 0,
      (int)wakeup_ts
    );
    wakeup_schedule_next();
    return true;
  }

  return false;
}
