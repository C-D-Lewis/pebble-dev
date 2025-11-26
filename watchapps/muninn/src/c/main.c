#include <pebble.h>

#include "config.h"

#include "modules/data.h"
#include "modules/wakeup.h"

#include "windows/main_window.h"
#include "windows/stat_window.h"
#include "windows/message_window.h"

#if PBL_API_EXISTS(app_glance_reload)
static void app_glance_callback(AppGlanceReloadSession *session, size_t limit, void *context) {
  if (limit < 1) return;

  static char s_buffer[32];
  if (data_calculate_days_remaining() == DATA_EMPTY) {
    snprintf(
      s_buffer,
      sizeof(s_buffer),
      "No data yet"
    );
  } else {
    snprintf(
      s_buffer,
      sizeof(s_buffer),
      "About %d days left",
      data_calculate_days_remaining()
    );
  }

  const AppGlanceSlice slice = (AppGlanceSlice) {
    .layout = {
      .subtitle_template_string = &s_buffer[0]
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };
  const AppGlanceResult result = app_glance_add_slice(session, slice);
  if (result != APP_GLANCE_RESULT_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
  }
}
#endif

static void battery_handler(BatteryChargeState state) {
  // Went up
  if (state.charge_percent > data_get_last_charge_perc()) {
    // Maintain charge level if it's going up
    data_set_last_charge_perc(state.charge_percent);
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
      message_window_push(MSG_WELCOME);
      data_set_seen_first_launch();
    }
  }
}

static void deinit() {
  data_deinit();

#if PBL_API_EXISTS(app_glance_reload)
  app_glance_reload(app_glance_callback, NULL);
#endif
}

int main() {
  init();
  app_event_loop();
  deinit();
}
