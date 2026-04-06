#pragma once

#include <pebble.h>

void data_set_current_temp(int temp);
void data_set_current_code(int code);
void data_set_temp_arr(char* temp_arr);
void data_set_precip_arr(char* precip_arr);
void data_set_code_arr(char* code_arr);

int data_get_current_temp();
int data_get_current_code();
char* data_get_temp_arr();
char* data_get_precip_arr();
char* data_get_code_arr();

GColor data_get_weather_color(int code);
int data_get_strarr_value(char *arr, int hour);
GColor data_get_temp_color(int temp);

