#include "wakeup.h"

void wakeup_unschedule() {
  wakeup_cancel_all();
  data_set_wakeup_id(STATUS_EMPTY);
}

void wakeup_schedule_next() {
  // We only want one - this one
  wakeup_unschedule();

  const time_t ts_now = time(NULL);

#if defined(WAKEUP_NEXT_MINUTE)
  // For faster testing of wakeups
  const time_t future = ts_now + 60;
#else
  // Find next interval hour based on WAKEUP_MOD_H
  const struct tm *now = localtime(&ts_now);
  const int hours_rem = WAKEUP_MOD_H - (now->tm_hour % WAKEUP_MOD_H);

  // Ensure it's exactly an interval of WAKEUP_MOD_H
  struct tm tm_future = *now;
  tm_future.tm_hour += hours_rem;
  tm_future.tm_min = 0;
  // Tiny extra offset in case weird things happen exactly on the hour
  tm_future.tm_sec = 5;
  const time_t future = mktime(&tm_future);
#endif

  APP_LOG(APP_LOG_LEVEL_INFO, "future %d", (int)future);
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

  data_set_wakeup_id(id);
  APP_LOG(APP_LOG_LEVEL_INFO, "Scheduled wakeup: %d", id);
}

void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  // We popped
  data_set_wakeup_id(STATUS_EMPTY);

  // Did we wake too early? Seen on PT2
  const time_t ts_now = time(NULL);
  const struct tm *now = localtime(&ts_now);
  if (now->tm_min > EXTRA_MINUTES_MAX) {
    // Try scheduling again
    wakeup_schedule_next();
    return;
  }

  const int last_charge_perc = data_get_last_charge_perc();
  const BatteryChargeState state = battery_state_service_peek();
  const int charge_percent = state.charge_percent;
  const int last_sample_time = data_get_last_sample_time();

  // First ever sample - nothing to compare to
  if (!util_is_not_status(last_sample_time)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");

    // Record state and wait for next time
    data_set_last_charge_perc(charge_percent);
    data_set_last_sample_time(ts_now);
  } else {
    const int time_diff_s = ts_now - last_sample_time;
    const int charge_diff = last_charge_perc - charge_percent;
    APP_LOG(APP_LOG_LEVEL_INFO, "Time diff: %d, Charge diff: %d", time_diff_s, charge_diff);

    // Turns out, in all cases below we note both of these anyway
    data_set_last_charge_perc(charge_percent);
    data_set_last_sample_time(ts_now);

    if (charge_diff < 0) {
      // Recently charged
      data_push_sample(
        charge_percent,
        last_sample_time,
        last_charge_perc,
        time_diff_s,
        charge_diff,
        STATUS_CHARGED
      );
    } else if (charge_diff == 0) {
      // No change since last sample - probably on charge or very short period
      data_push_sample(
        charge_percent,
        last_sample_time,
        last_charge_perc,
        time_diff_s,
        charge_diff,
        STATUS_NO_CHANGE
      );
    } else {
      // Calculate new daily discharge rate estimate!
      const int estimate = (charge_diff * SECONDS_PER_DAY) / time_diff_s;

      data_push_sample(
        charge_percent,
        last_sample_time,
        last_charge_perc,
        time_diff_s,
        charge_diff,
        estimate
      );
      APP_LOG(APP_LOG_LEVEL_INFO, "estimate: %d", estimate);
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

    vibes_double_pulse();
    alert_window_push(
      RESOURCE_ID_AWAKE,
      "Muninn advises the battery is below your chosen threshold.",
      true,
      false
    );
    return;
  }
  if (!is_low && ca_has_notified) data_set_ca_has_notified(false);

  // Rate is unusually high
  if (data_get_rate_is_elevated() && data_get_elevated_rate_alert()) {
    vibes_double_pulse();
    alert_window_push(
      RESOURCE_ID_AWAKE,
      "Muninn advises the battery is draining faster than usual.",
      true,
      false
    );
    return;
  }

  // One day left - show peristent alert
  const bool one_day_notified = data_get_one_day_notified();
  const bool one_day_left = data_calculate_days_remaining() <= 1;
  if (one_day_left && !one_day_notified) {
    data_set_one_day_notified(true);

    alert_window_push(
      RESOURCE_ID_AWAKE,
      "Muninn advises you may have one day remaining.",
      true,
      false
    );
    return;
  }
  if (!one_day_left && one_day_notified) data_set_one_day_notified(false);

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
  if (!util_is_not_status(wakeup_id)) return false;

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
