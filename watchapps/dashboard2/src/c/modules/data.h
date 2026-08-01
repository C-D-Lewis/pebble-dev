#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/main_window.h"

// NOTE: APPEND ONLY - Changing keys will affect existing users!
typedef enum {
  // App data key
  SK_PersistData = 1,

  // Max storage value used
  SK_Max = 51
} StorageKey;

// NOT persisted data
typedef struct {
  SyncState sync_state;
  int compat_protocol_version;
  char toggle_order[TOGGLES_STRLEN + 1];   // 2c x X toggles + null
  char device_name[32];
  char battery_perc[4];    // '100'
  char free_space[12];     // '999.99 GB'
  char free_space_perc[4]; // '100'

} AppState;

void data_init();
void data_deinit();

AppState* data_get_app_state();

#ifdef USE_TEST_DATA
void data_test_data_handler();
#endif

int data_get_toggles_length();

bool data_is_toggle_active(int index);
