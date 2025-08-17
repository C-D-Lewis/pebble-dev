#include <pebble.h>

#include "windows/main_window.h"

#include "modules/data.h"
#include "modules/comm.h"

static void init() {
  data_init();

  const uint32_t buffer_size = 256;
  comm_init(buffer_size, buffer_size);
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
