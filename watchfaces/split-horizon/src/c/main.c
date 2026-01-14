#include <pebble.h>

#include "windows/main_window.h"

#include "modules/data.h"
#include "modules/comm.h"
#include "modules/scalable.h"

static void init() {
  scalable_init();
  data_init();
  comm_init();

  main_window_push();
}

static void deinit() {
}

int main() {
  init();
  app_event_loop();
  deinit();
}
