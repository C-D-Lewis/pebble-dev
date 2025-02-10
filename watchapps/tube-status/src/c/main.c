#include <pebble.h>

#include "config.h"
#include "modules/data.h"
#include "modules/comm.h"

#include "common/splash_window.h"
  
static void init() {
  data_init();
  comm_init();
	splash_window_push();
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
