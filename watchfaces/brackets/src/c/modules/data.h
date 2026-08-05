#pragma once

#include <pebble.h>

typedef struct {
  bool second_tick;
  bool battery_and_bluetooth;
  char complication_type[16];

  GColor color_background;
  GColor color_brackets;
  GColor color_datetime;
  GColor color_complications;
} PersistData;

typedef struct {
  char weather_status[16];
} AppState;

typedef enum {
  SK_PersistData = 1
} StorageKeys;

// Same as Clay config
#define COMPLICATION_TYPE_WEATHER "weather"
#define COMPLICATION_TYPE_STEP_COUNT "step_count"
#define COMPLICATION_TYPE_NONE "none"

void data_init();
void data_deinit();

PersistData* data_get_persist_data();

AppState* data_get_app_state();
