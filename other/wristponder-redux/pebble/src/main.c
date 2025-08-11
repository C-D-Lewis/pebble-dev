#include <pebble.h>

#include "common/dialog_window.h"
#include "common/splash_window.h"

#include "modules/data.h"
#include "modules/comm.h"
#include "config.h"

static AppTimer *s_timeout_timer;

static void continue_handler(void *context) {
  comm_request_initial_contacts();
}

static void version_check_callback(bool successful) {
  if(successful) {
    // Cancel timeout
    if(s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
      s_timeout_timer = NULL;
    }

    // Delay befor next window
    app_timer_register(100, continue_handler, NULL);
  } else {
    // Inform off incorrect version
    dialog_window_push("Watchapp must be updated from Android app.");
    splash_window_pop();
  }
}

static void timeout_handler(void *context) {
  // Retry
  version_check(COMPATIBLE_VERSION, version_check_callback);
  s_timeout_timer = app_timer_register(SPLASH_WINDOW_TIMEOUT_DELAY, timeout_handler, NULL);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "timeout!");
}

/************************************ App *************************************/

static void init() {
  data_init();

  const int inbox = 3100;    // 30 x 100 char responses, less than 30 contacts at once
  const int outbox = DICT_BUFFER + 32;  // Dictation max size plus a little extra
  comm_init(inbox, outbox);

  if(!connection_service_peek_pebble_app_connection()) {
    dialog_window_push("Phone is disconnected.");
    return;
  }

  splash_window_push();

#if defined(TEST)
  version_check_callback(true);
#else
  timeout_handler(NULL);
#endif
}

static void deinit() {
  data_deinit();
  comm_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
