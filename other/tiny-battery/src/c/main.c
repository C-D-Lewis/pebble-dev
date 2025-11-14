#include <pebble.h>

#include "modules/data.h"
#include "modules/wakeup.h"

#include "windows/stat_window.h"
#include "windows/main_window.h"

static void battery_handler(BatteryChargeState state) {
  // Un/plugged while app is open
  if (data_get_was_plugged() && !state.is_plugged) {
    const time_t now = time(NULL);
    data_set_discharge_start_time(now);
  }

  data_set_was_plugged(state.is_plugged);

  stat_window_update_data();
}

static void init() {
  data_init();

  // In case an event comes when the app is open
  wakeup_service_subscribe(wakeup_handler);

  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    main_window_push();
  }

  // If app is open, get more accurate battery data
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  // TODO: Detect if we missed a wakeup if the watch was off?
  //       Notification or auto-re-enable?
}

static void deinit() {
  data_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
