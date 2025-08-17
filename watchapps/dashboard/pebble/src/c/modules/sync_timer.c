#include "sync_timer.h"

static AppTimer *s_timer;
static int s_interval_ms;

static void timer_handler(void *context);

static void re_register() {
  s_timer = app_timer_register(s_interval_ms, timer_handler, NULL);
}

static void timer_handler(void *context) {
  s_timer = NULL;
  comm_request_all();
  re_register();
  if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Resynchronizing...");
}

void sync_timer_begin(int interval_s) {
  s_interval_ms = interval_s * 1000;
  re_register();
}
