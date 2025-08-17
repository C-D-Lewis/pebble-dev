#include <pebble.h>

#include "config.h"

#include "modules/data.h"
#include "modules/comm.h"
#include "modules/sync_timer.h"

#include "windows/splash_window.h"
#include "windows/list_window.h"

// --------------------------------- AppGlance ---------------------------------

#if PBL_API_EXISTS(app_glance_reload)
static void app_glance_callback(AppGlanceReloadSession *session, size_t limit, void *context) {
  if(limit < 1) {
    return;
  }

  // Find next upcoming Event
  Event events[MAX_WAKEUPS];
  int next_index = -1;
  time_t next_timestamp = 2147483647; // Max int32
  for(int i = 0; i < MAX_WAKEUPS; i++) {
    if(!persist_exists(i)) {
      continue;
    }
    
    persist_read_data(i, &events[i], sizeof(Event));
    time_t timestamp;
    wakeup_query(events[i].wakeup_id, &timestamp);

    if(timestamp < next_timestamp) {
      next_timestamp = timestamp;
      next_index = i;
    }
  }

  char buffer[64];
  if(next_index == -1) {
    // No events found
    snprintf(buffer, sizeof(buffer), "No upcoming events");
  } else {
    int minute = events[next_index].minute;
    char min_buff[8];
    if(minute < 10) {
      snprintf(min_buff, sizeof(min_buff), "0%d", minute);
    } else {
      snprintf(min_buff, sizeof(min_buff), "%d", minute);
    }
    snprintf(buffer, sizeof(buffer), "Next event: %s %s at %d:%s", 
      list_window_get_type_string(events[next_index].type), 
      wakeup_window_get_state_string(events[next_index].state), 
      events[next_index].hour, min_buff);

    time_t timestamp;
    wakeup_query(events[next_index].wakeup_id, &timestamp);
  }
  const AppGlanceSlice slice = (AppGlanceSlice) {
    .layout = {
      .subtitle_template_string = &buffer[0]
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };
  app_glance_add_slice(session, slice);
}
#endif

// ---------------------------------- Wakeup -----------------------------------

static void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  vibes_double_pulse();

  // Load event
  Event e;
  persist_read_data(cookie, &e, sizeof(Event));

  // Wakeup found, carry out action
  if(comm_check_bt("Dashboard event failed: watch was disconnected.")) {
    comm_request_toggle(e.type, e.state);
  }

  // Re-schedule for same time tomorrow
  time_t timestamp = clock_to_timestamp(schedule_window_get_tomorrow_weekday(), e.hour, e.minute);
  int id = wakeup_schedule(timestamp, cookie, false);

  // Check
  if(wakeup_query(id, &timestamp)) {
    // Store backing data
    e.wakeup_id = id;
    persist_write_data(cookie, &e, sizeof(Event));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Wakeup recall failed %d", id);
    dialog_window_push("Failed to re-schedule wakeup for tomorrow");
  }

  // Show UI
  wakeup_window_push(e.type, e.state);
}

static void begin_handler(void *context) {
  if(TEST) {
    test_populate_data();
    return;
  }

  if(!comm_check_bt("Watch disconnected. Reconnect to continue.")) {
    return;
  }

  // Connect to real device
  sync_timer_begin(SYNC_TIMER_INTERVAL_S);  // Update regularly while open
  comm_request_all();                       // Response will contain everything, and use comm
                                            // module timeout mechanism
}

// ------------------------------------ App ------------------------------------

static void init() {
  comm_init(BUFFER_SIZE_IN, BUFFER_SIZE_OUT);
  data_init();
  wakeup_service_subscribe(wakeup_handler);

  if(launch_reason() == APP_LAUNCH_WAKEUP) {
    // The app was started by a wakeup
    WakeupId id = 0;
    int32_t reason = 0;

    // Get details and handle the wakeup
    wakeup_get_launch_event(&id, &reason);
    wakeup_handler(id, reason);
  } else {
    // No wakeup, launch normally
    splash_window_push();
    app_timer_register(INITIAL_DELAY, begin_handler, NULL);
  }
}

static void deinit() {
  data_deinit();

#if PBL_API_EXISTS(app_glance_reload)
  app_glance_reload(app_glance_callback, NULL);
#endif
}

int main() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Dashboard v%s", VERSION);

  init();
  app_event_loop();
  deinit();
}
