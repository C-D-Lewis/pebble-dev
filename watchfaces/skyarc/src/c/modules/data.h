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
  char temp_arr[STR_ARR_SIZE];
  char precip_arr[STR_ARR_SIZE];
  char code_arr[STR_ARR_SIZE];
} AppState;

typedef struct {
  char temp_unit[4];
} PersistData;

void data_init();
void data_deinit();

AppState* data_get_app_state();
PersistData* data_get_persist_data();

GColor data_get_weather_color(int code);
int data_get_strarr_value(char *arr, int hour);
GColor data_get_temp_color(int temp);
char* data_get_weather_str(int code);
GColor data_get_precip_color(int precip_chance);
