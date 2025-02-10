#include <pebble.h>

#include "windows/main_window.h"

static void init() {
  main_window_push();
}

static void deinit() { }

int main() {
  init();
  app_event_loop();
  deinit();
}