#include <pebble.h>

#include "config.h"

#include "modules/comm.h"
#include "modules/data.h"
#include "modules/wakeup.h"
#include "modules/glance.h"
#include "modules/scalable.h"
#include "modules/bitmaps.h"

#include "windows/main_window.h"
#include "windows/message_window.h"

static void init() {
  AppState *app_state = data_get_app_state();
  PersistData *persist_data = data_get_persist_data();

  data_init();
  comm_init();
  scalable_init();

  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
    return;
  }
  

  if (wakeup_handle_missed()) {
    // Test data won't have valid wakeup ID
#if !defined(USE_TEST_DATA)
    app_state->missed_sample = true;
#endif
  }

  main_window_push();

  // In case an event comes when the app is open
  wakeup_service_subscribe(wakeup_handler);

  const bool first_launch = !persist_data->seen_first_launch;
  if (first_launch) {
// VERY LOW MEMORY
#ifndef PBL_PLATFORM_APLITE
    message_window_push(
      "Welcome to Muninn!\n\nEstimates will appear after 3 samples.\n\nLaunch me if the watch is off when a sample is missed.",
      false,
      false
    );
#endif
    persist_data->seen_first_launch = true;
  }
}

static void deinit() {
  data_deinit();
  bitmaps_destroy_all();

#if PBL_API_EXISTS(app_glance_reload)
  app_glance_reload(glance_handler, NULL);
#endif
}

int main() {
  init();
  app_event_loop();
  deinit();
}
