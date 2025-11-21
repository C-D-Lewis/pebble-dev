#include <pebble.h>

#include "config.h"

#include "modules/data.h"
#include "modules/wakeup.h"

#include "windows/main_window.h"
#include "windows/stat_window.h"
#include "windows/message_window.h"

char *welcome_text = "Welcome to Muninn!\n\nHe will track your battery over time.\n\nPlease launch after each reboot.";

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

static void init() {
  data_init();

  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    const bool missed = handle_missed_wakeup();
    const bool first_launch = !data_get_seen_first_launch();

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

    if (first_launch) {
      message_window_push(welcome_text);
      data_set_seen_first_launch();
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
