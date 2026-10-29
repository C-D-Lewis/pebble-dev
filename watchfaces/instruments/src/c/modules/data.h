#pragma once

#include <pebble.h>

#include "../config.h"

typedef enum {
  SK_PersistData = 10,
} StorageKeys;

typedef struct {
  int current_temp;
  int current_code;
  char sunrise[8];
  char sunset[8];
} AppState;

typedef struct {
  char temp_unit[4];  // C or F
  char theme[8];      // BLUE, RED, etc.
  char slot_1[16];    // Complication slot 1
  char slot_2[16];    // Complication slot 2
  char slot_3[16];    // Complication slot 3
  char slot_4[16];    // Complication slot 4
  char step_goal[8];  // Steps goal
} PersistData;

void data_init();
void data_deinit();

AppState* data_get_app_state();
PersistData* data_get_persist_data();

uint32_t data_get_weather_icon_id();

GColor data_get_theme_day_color();
GColor data_get_theme_night_color();

bool data_is_health_used();
