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
  int current_humidity_perc;
  int current_wind_kmh;
  char temp_arr[STR_ARR_SIZE];
  char precip_arr[STR_ARR_SIZE];
  char code_arr[STR_ARR_SIZE];
} AppState;

typedef struct {
  char temp_unit[4];         // C or F
  char wind_unit[8];         // MPH or KPH
  char animations[8];        // 'true' or 'false' - use string for the unset/false state
  char color_bg[32];         // Name of selected color
  int tap_timeout;
  char cloud_render_mode[8]; // STRIPED or SOLID
} PersistData;

void data_init();
void data_deinit();

AppState* data_get_app_state();
PersistData* data_get_persist_data();
int data_get_min_temp();
int data_get_max_temp();
GColor data_get_bg_color();

GColor data_get_weather_color(int code);
int data_get_strarr_value(char *arr, int hour);
GColor data_get_temp_color(int temp);
char* data_get_weather_str(int code);
GColor data_get_precip_color(int precip_chance);
