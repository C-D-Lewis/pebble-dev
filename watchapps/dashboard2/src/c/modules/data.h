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

// Persisted app data - be careful changing this!
// To remove fields, add '__' prefix and leave in place (for struct packing)
typedef struct {
  // Nothing yet

  // Singleton, adding new fields OK, removing from middle is NOT OK
} PersistData;

// NOT persisted data
typedef struct {
  int compat_protocol_version;
  char toggle_order[33];   // 2c x 16 toggles + null
  char device_name[32];
  char battery_perc[4];    // '100'
  char free_space[12];     // '999.99 GB'
  char free_space_perc[4]; // '100'

} AppState;

void data_init();
void data_deinit();

PersistData* data_get_persist_data();
AppState* data_get_app_state();

#ifdef USE_TEST_DATA
void data_test_data_handler();
#endif