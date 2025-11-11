#include <pebble.h>

#include "modules/data.h"
#include "modules/wakeup.h"

#include "windows/main_window.h"

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
}

static void deinit() {
  data_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
