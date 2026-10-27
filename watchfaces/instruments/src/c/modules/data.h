#pragma once

#include <pebble.h>

#include "../config.h"

typedef enum {
  SK_PersistData = 10,
} StorageKeys;

typedef struct {
  int current_temp;
  int current_code;
} AppState;

typedef struct {
  char temp_unit[4]; // C or F
} PersistData;

void data_init();
void data_deinit();

AppState* data_get_app_state();
PersistData* data_get_persist_data();

uint32_t data_get_weather_icon_id(int code);
