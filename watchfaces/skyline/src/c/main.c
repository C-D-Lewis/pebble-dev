#include <pebble.h>

#include "windows/main_window.h"

#include "modules/comm.h"
#include "modules/scalable.h"

static void init() {
  comm_init(256, 256);
  scalable_init();

  main_window_push();
}

static void deinit() {
}

int main() {
  init();
  app_event_loop();
  deinit();
}
