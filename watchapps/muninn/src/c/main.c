#include <pebble.h>

#include "config.h"

#include "modules/data.h"
#include "modules/wakeup.h"
#include "modules/glance.h"

#include "windows/main_window.h"
#include "windows/stat_window.h"
#include "windows/message_window.h"

static void battery_handler(BatteryChargeState state) {
  // Went up, will only go down from there
  if (state.charge_percent > data_get_last_charge_perc()) {
    // Maintain charge level if it's going up
    data_set_last_charge_perc(state.charge_percent);
  }

  stat_window_update_data();
}

static bool handle_missed_wakeup() {
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

static void init() {
  data_init();

  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
    return;
  }
  
  const bool missed = handle_missed_wakeup();
  const bool first_launch = !data_get_seen_first_launch();

  main_window_push();

  // In case an event comes when the app is open
  wakeup_service_subscribe(wakeup_handler);

  // If app is open, get more accurate battery data
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  if (missed) {
#if !defined(USE_TEST_DATA)
    alert_window_push(
      RESOURCE_ID_ASLEEP,
      "Muninn missed a sample, but will continue.",
      true,
      false
    );
#endif
  }

  if (first_launch) {
    message_window_push(MSG_WELCOME);
    data_set_seen_first_launch();
  }
}

static void deinit() {
  data_deinit();

#if PBL_API_EXISTS(app_glance_reload)
  app_glance_reload(glance_handler, NULL);
#endif
}

int main() {
  init();
  app_event_loop();
  deinit();
}
