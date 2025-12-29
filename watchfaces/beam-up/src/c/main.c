#include <pebble.h>

#include "windows/main_window.h"

#include "modules/data.h"
#include "modules/comm.h"
#include "modules/scalable.h"

static void init() {
  data_init();
  scalable_init();

  comm_init(128, 128);
  setlocale(LC_ALL, "");

  main_window_push();
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
