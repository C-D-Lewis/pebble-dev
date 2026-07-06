#include <pebble.h>

#include "config.h"
#include "modules/bitmaps.h"
#include "modules/comm.h"
#include "modules/data.h"
#include "modules/scalable.h"

#include "windows/main_window.h"
  
static void init() {
  data_init();
  comm_init();
  scalable_init();

	main_window_push();
}

static void deinit() {
  data_deinit();
  comm_deinit();
  bitmaps_destroy_all();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
