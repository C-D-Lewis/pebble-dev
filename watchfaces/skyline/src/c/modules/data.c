#include "data.h"

#define STR_ARR_SIZE (24 * 2)

static int s_current_temp, s_current_code;
static char s_temp_arr[STR_ARR_SIZE];
static char s_precip_arr[STR_ARR_SIZE];
static char s_code_arr[STR_ARR_SIZE];

void data_set_current_temp(int temp) {
  s_current_temp = temp;
}

void data_set_current_code(int code) {
  s_current_code = code;
}

void data_set_temp_arr(char *temp_arr) {
  snprintf(s_temp_arr, STR_ARR_SIZE, "%s", temp_arr);
}

void data_set_precip_arr(char *precip_arr) {
  snprintf(s_precip_arr, STR_ARR_SIZE, "%s", precip_arr);
}

void data_set_code_arr(char *code_arr) {
  snprintf(s_code_arr, STR_ARR_SIZE, "%s", code_arr);
}

int data_get_current_temp() {
  return s_current_temp;
}

int data_get_current_code() {
  return s_current_code;
}

char *data_get_temp_arr() {
  return s_temp_arr;
}

char* data_get_precip_arr() {
  return s_precip_arr;
}

char* data_get_code_arr() {
  return s_code_arr;
}
