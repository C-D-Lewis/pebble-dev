#pragma once

#include <pebble.h>

#include "../config.h"

typedef enum {
  SK_TempUnit = 10,
} StorageKeys;

void data_init();
void data_deinit();

void data_set_current_temp(int temp);
void data_set_current_code(int code);
void data_set_temp_arr(char* temp_arr);
void data_set_precip_arr(char* precip_arr);
void data_set_code_arr(char* code_arr);
void data_set_temp_unit(char* temp_unit);

int data_get_current_temp();
int data_get_current_code();
char* data_get_temp_arr();
char* data_get_precip_arr();
char* data_get_code_arr();
char* data_get_temp_unit();

GColor data_get_weather_color(int code);
int data_get_strarr_value(char *arr, int hour);
GColor data_get_temp_color(int temp);
char* data_get_weather_str(int code);
GColor data_get_precip_color(int precip_chance);
