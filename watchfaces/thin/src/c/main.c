#include <pebble.h>

#include "modules/comm.h"
#include "modules/data.h"
#include "windows/main_window.h"

static void init() {
  setlocale(LC_ALL, "");
  comm_init();
  data_init();
  
  if (data_get(DataKeySteps)) {
    health_init();
  }
  
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
