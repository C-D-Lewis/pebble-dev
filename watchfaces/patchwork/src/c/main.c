#include <pebble.h>

#include "modules/data.h"
#include "modules/comm.h"

#include "windows/main_window.h"

static void init() {
  data_init();
  comm_init(64, 64);

  main_window_push();
}

static void deinit() {
  data_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
