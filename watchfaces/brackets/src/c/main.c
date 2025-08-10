#include <pebble.h>

#include "modules/comm.h"
#include "modules/data.h"

#include "windows/main_window.h"

static void init() {
  comm_init();
  data_init();

  main_window_push();
}

static void deinit() { }

int main() {
  init();
  app_event_loop();
  deinit();
}
