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
  char precip_arr[STR_ARR_SIZE];
  char code_arr[STR_ARR_SIZE];
  int temp_low;
  int temp_high;
  int moon_phase;
  int uv_index_max;
  int phone_battery;
} AppState;

typedef struct {
  char temp_unit[4];  // C or F
  char theme[8];      // BLUE, RED, etc.
  char slot_1[16];    // Complication slot 1
  char slot_2[16];    // Complication slot 2
  char slot_3[16];    // Complication slot 3
  char slot_4[16];    // Complication slot 4
  char slot_5[16];    // Complication slot 5
  char slot_6[16];    // Complication slot 6
  char step_goal[8];  // Steps goal
} PersistData;

void data_init();
void data_deinit();

void data_reset_app_state();

AppState* data_get_app_state();
PersistData* data_get_persist_data();

GColor data_get_theme_day_color();
GColor data_get_theme_night_color();
GColor data_get_precip_color(int precip_chance);
GColor data_get_weather_color(int code);

uint32_t data_get_weather_icon_id();

char* data_get_weather_str(int code);

bool data_is_health_used();

int data_get_strarr_value(char *arr, int hour);

bool data_is_weather_valid();

int data_get_complication_count();
char* data_get_nth_complication_type(int n);
