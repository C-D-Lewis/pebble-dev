#include <pebble.h>

#include "modules/scalable.h"

#include "windows/main_window.h"

static void init() {
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
