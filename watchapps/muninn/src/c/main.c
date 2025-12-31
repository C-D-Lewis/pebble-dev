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
#include "windows/alert_window.h"
#include "windows/graph_window.h"

static void init() {
  // Temporary - to be removed a few versions after 1.12.0
  const int old_appdata_key = 0;
  const bool notify_wipe = persist_exists(old_appdata_key) && !persist_exists(SK_Migration_1);

  data_init();
  comm_init();
  scalable_init();

  if (launch_reason() == APP_LAUNCH_WAKEUP && !notify_wipe) {
    WakeupId id = 0;
    int32_t reason = 0;
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
    return;
  }
  
  const bool missed = wakeup_handle_missed();
  const bool first_launch = !data_get_seen_first_launch();

  main_window_push();
  graph_window_push();

  if (notify_wipe) {
    alert_window_push(
      RESOURCE_ID_AWAKE,
      "Data reset to allow some new features to work correctly.",
      true,
      false
    );
  }

  if (data_get_push_timeline_pins()) {
    // Try and push a pin
    comm_push_timeline_pins();
  }

  // In case an event comes when the app is open
  wakeup_service_subscribe(wakeup_handler);

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
