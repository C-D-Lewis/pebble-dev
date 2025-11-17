#include <pebble.h>

#include "config.h"

#include "modules/data.h"
#include "modules/wakeup.h"

#include "windows/main_window.h"
#include "windows/stat_window.h"

static void battery_handler(BatteryChargeState state) {
  // Un/plugged while app is open
  if (data_get_was_plugged() && !state.is_plugged) {
    const time_t now = time(NULL);
    data_set_discharge_start_time(now);
  }

  data_set_was_plugged(state.is_plugged);

  stat_window_update_data();
}

static bool handle_missed_wakeup() {
  const int wakeup_id = data_get_wakeup_id();
  if (wakeup_id == DATA_EMPTY) return false;

  time_t now = time(NULL);
  time_t wakeup_ts;
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);

  // Doesn't exist or is too long ago, reschedule it
  if (!found || (now - wakeup_ts) > (WAKEUP_H * SECONDS_PER_HOUR)) {
    wakeup_schedule_next();
    APP_LOG(
      APP_LOG_LEVEL_INFO,
      "Re-scheduled stale wakeup: %d %d -> %d",
      wakeup_id,
      found ? 1 : 0,
      (int)wakeup_ts
    );
    return true;
  }

  return false;
}

static void init() {
  data_init();

  const bool missed = handle_missed_wakeup();

  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    main_window_push();

    // In case an event comes when the app is open
    wakeup_service_subscribe(wakeup_handler);

    // If app is open, get more accurate battery data
    battery_state_service_subscribe(battery_handler);
    battery_handler(battery_state_service_peek());

    if (missed) {
      alert_window_push(
        RESOURCE_ID_ASLEEP,
        "Muninn missed a sample, but will continue.",
        true,
        false
      );
    }
  }
}

static void deinit() {
  data_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
