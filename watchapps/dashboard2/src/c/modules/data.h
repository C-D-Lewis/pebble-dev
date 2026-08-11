#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/main_window.h"

// NOT persisted data
typedef struct {
  SyncState sync_state;
  int compat_protocol_version;
  char toggle_order[TOGGLES_STRLEN + 1];   // 2c x X toggles + null
  char device_name[32];
  char battery_perc[4];    // '100'
  char free_space[12];     // '999.99 GB'
  char free_space_perc[4]; // '100'
  char next_alarm[16];     // 'Wed 08:00'
  bool is_charging;

} AppState;

void data_init();
void data_deinit();

AppState* data_get_app_state();

bool data_is_toggle_active(int index);

#ifdef USE_TEST_DATA
void data_test_data_handler();
#endif
