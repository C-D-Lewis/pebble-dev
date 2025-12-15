#include <pebble.h>

#include "config.h"
#include "modules/data.h"
#include "modules/comm.h"
#include "modules/scalable.h"

#include "windows/common/splash_window.h"
  
static void init() {
  data_init();
  comm_init();
  scalable_init();

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
