#include <pebble.h>

#include "modules/comm.h"
#include "modules/data.h"

#include "windows/main_window.h"

static void init() {
  comm_init(128, 128);
  data_init();

  main_window_push();
}

static void deinit() {
  comm_deinit();
  data_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
