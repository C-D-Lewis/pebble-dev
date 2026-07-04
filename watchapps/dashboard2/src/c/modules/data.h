#pragma once

#include <pebble.h>

#include "../config.h"

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
  char test_message[32];

  // Singleton, adding new fields OK, removing from middle IS OK
} AppState;

void data_init();
void data_deinit();

PersistData* data_get_persist_data();
AppState* data_get_app_state();