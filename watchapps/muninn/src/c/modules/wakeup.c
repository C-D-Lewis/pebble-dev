#include "wakeup.h"

void wakeup_unschedule() {
  PersistData *persist_data = data_get_persist_data();

  wakeup_cancel_all();
  persist_data->wakeup_id = STATUS_EMPTY;
}

void wakeup_schedule_next() {
  PersistData *persist_data = data_get_persist_data();

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
  tm_future.tm_sec = 0;
  const time_t future = mktime(&tm_future);
#endif

// APP_LOG(APP_LOG_LEVEL_INFO, "future %d", (int)future);
  int id = wakeup_schedule(future, 0, true);
#if defined(TEST_COLLISION)
  // To test collision rescheduling
  id = wakeup_schedule(future, 0, true);
#endif

  // Failed for some reason
  if (id < 0) {
    // Try again in the future if a collision with another app
    int extra_mins = 1;
    while (id < 0 && extra_mins <= EXTRA_MINUTES_MAX) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "E_RANGE, trying again with +%dm", extra_mins);
      id = wakeup_schedule(future + (extra_mins * 60), 0, true);
      extra_mins++;
    }
  }

  // If still failed
  if (id < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "schedule fail");

    static char err_buff[64];
    snprintf(err_buff, sizeof(err_buff), "Failed to schedule wakeup: %d", id);
    data_set_error(err_buff);
    return;
  }

  persist_data->wakeup_id = id;
// APP_LOG(APP_LOG_LEVEL_INFO, "Scheduled wakeup: %d", id);
}

void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  PersistData *persist_data = data_get_persist_data();

  // We popped
  persist_data->wakeup_id = STATUS_EMPTY;

  // Did we wake too early? Seen on PT2
  const time_t ts_now = time(NULL);
  const struct tm *now = localtime(&ts_now);
  if (now->tm_min > EXTRA_MINUTES_MAX) {
    // Try scheduling again
    wakeup_schedule_next();
    return;
  }

  const int last_charge_perc = persist_data->last_charge_perc;
  const BatteryChargeState state = battery_state_service_peek();
  const int charge_percent = state.charge_percent;
  const int last_sample_time = persist_data->last_sample_time;

  int result = STATUS_EMPTY;

  // First ever sample - nothing to compare to
  if (!util_is_not_status(last_sample_time)) {
  // APP_LOG(APP_LOG_LEVEL_INFO, "First sample!");

    // Record state and wait for next time
    persist_data->last_charge_perc = charge_percent;
    persist_data->last_sample_time = ts_now;
  } else {
    const int time_diff_s = ts_now - last_sample_time;
    const int discharge_perc = last_charge_perc - charge_percent;
  // APP_LOG(APP_LOG_LEVEL_INFO, "Time diff: %d, Charge diff: %d", time_diff_s, discharge_perc);

    const bool battery_bumped = discharge_perc < 0 && discharge_perc > -MIN_CHARGE_AMOUNT;
    if (!battery_bumped) {
      // If a significant event, note the new conditions
      persist_data->last_charge_perc = charge_percent;
      persist_data->last_sample_time = ts_now;
    }

    if (discharge_perc < 0) {
      // Can't combine with the 'if' above due to the 'else' otherwise being taken
      if (!battery_bumped) {
        // Recently charged by user a significant amount
        result = STATUS_CHARGED;
        persist_data->last_charge_time = ts_now;
      }
    } else if (discharge_perc == 0) {
      // No change since last sample - probably on full-charge or very low drain
      result = STATUS_NO_CHANGE;
    } else {
      // Calculate new daily discharge rate estimate!
      result = (discharge_perc * SECONDS_PER_DAY) / time_diff_s;
    // APP_LOG(APP_LOG_LEVEL_INFO, "estimate: %d", result);
    }

    if (result != STATUS_EMPTY) {
      data_push_sample(
        charge_percent,
        last_sample_time,
        last_charge_perc,
        time_diff_s,
        discharge_perc,
        result
      );
    }
  }

  wakeup_schedule_next();

  // Should we advise battery is low?
  const int alert_level = persist_data->custom_alert_level;
  const bool is_low = alert_level != AL_OFF && last_charge_perc <= alert_level;
  const bool ca_has_notified = persist_data->ca_has_notified;

  // If we have data, and it's lower, and we haven't notified
  if (data_get_valid_samples_count() > MIN_SAMPLES && is_low && !ca_has_notified) {
    persist_data->ca_has_notified = true;

    if (!quiet_time_is_active()) vibes_double_pulse();
    message_window_push("Muninn advises the battery is below your chosen threshold.", true, false);
    return;
  }
  if (!is_low && ca_has_notified) persist_data->ca_has_notified = false;

  // Rate is unusually high
  if (data_get_rate_is_elevated() && persist_data->elevated_rate_alert) {
    if (!quiet_time_is_active()) vibes_double_pulse();
    message_window_push("Muninn advises the battery is draining faster than usual.", true, false);
    return;
  }

  // One day left - show peristent alert
  const bool one_day_notified = persist_data->one_day_notified;
  const int days_remaining = data_calculate_days_remaining();
  const bool one_day_left = util_is_not_status(days_remaining) && days_remaining <= 1;
  if (persist_data->one_day_alert && one_day_left && !one_day_notified) {
    persist_data->one_day_notified = true;

    message_window_push("Muninn advises you have one day remaining.", true, false);
    return;
  }
  if (!one_day_left && one_day_notified) persist_data->one_day_notified = false;

  // Main display on wakeup
  const bool do_vibe = persist_data->vibe_on_sample;
  if (result != STATUS_EMPTY) {
    // Tell the user if a sample was taken
    message_window_push("Muninn is taking a note.", do_vibe, true);
  } else {
    message_window_push("Muninn chose to wait some more.", do_vibe, true);
  }
}

bool wakeup_handle_missed() {
  PersistData *persist_data = data_get_persist_data();

  const int wakeup_id = persist_data->wakeup_id;
  if (!util_is_not_status(wakeup_id)) return false;

  time_t wakeup_ts;
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);

  // Doesn't exist or is too long ago, reschedule it
  if (!found || (time(NULL) - wakeup_ts) > (WAKEUP_MOD_H * SECONDS_PER_HOUR)) {
    // APP_LOG(
    //   APP_LOG_LEVEL_INFO,
    //   "Missed w: %d %d %d",
    //   wakeup_id,
    //   found ? 1 : 0,
    //   (int)wakeup_ts
    // );
    wakeup_schedule_next();
    return true;
  }

  return false;
}
