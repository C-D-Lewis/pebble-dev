#include "data.h"

#define STR_ARR_SIZE 49
#define INIT_MIN_TEMP 999
#define INIT_MAX_TEMP -999

static int s_current_temp, s_current_code;
static char s_temp_arr[STR_ARR_SIZE];
static char s_code_arr[STR_ARR_SIZE];
static int s_min_temp = INIT_MIN_TEMP, s_max_temp = INIT_MAX_TEMP;

/***************************** Getters / setters ******************************/

void data_set_current_temp(int temp) {
  s_current_temp = temp;
}

void data_set_current_code(int code) {
  s_current_code = code;
}

void data_set_temp_arr(char *temp_arr) {
  snprintf(s_temp_arr, sizeof(s_temp_arr), "%s", temp_arr);
}

void data_set_code_arr(char *code_arr) {
  snprintf(s_code_arr, sizeof(s_code_arr), "%s", code_arr);
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

char* data_get_code_arr() {
  return s_code_arr;
}

/********************************** Methods ***********************************/

GColor data_get_weather_color(int code) {
  switch (code) {
    case 0:
    case 1: // Mainly clear
      return GColorClear;
    case 3: // Overcast
      return GColorDarkGray;
    case 2: // Partly cloudy
    case 45: // Fog
    case 48: // Depositing rime fog
      return GColorLightGray;
    case 51: // Light Drizzle
    case 53: // Moderate Drizzle
    case 55: // Dense Drizzle
    case 56: // Light Freezing Drizzle
    case 57: // Dense Freezing Drizzle
    case 61: // Slight Rain
    case 66: // Light Freezing Rain
    case 80: // Slight Rain showers
      return GColorBlueMoon;
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 67: // Heavy Freezing Rain
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
      return GColorBlue;
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
      return GColorWhite;
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return GColorYellow;
    default:
      return GColorClear;
  }
};

/**
 * Arrays are two chars per item, 24 items.
 *
 *   "TEMPS": "121211101009080910101112121212121211100807060606",
 *   "PRECIP": "000000000000000000020713161408040201000000000000",
 *   "CODES": "030303030303030302030202030202030302000100000000"
 */
int data_get_strarr_value(char *arr, int hour) {
  const int index = hour * 2; // Two chars per code
  const char value[3] = {arr[index], arr[index + 1], '\0'};
  // APP_LOG(APP_LOG_LEVEL_INFO, "arr %s", arr);
  // APP_LOG(APP_LOG_LEVEL_INFO, "hour %d value %s", (int)hour, value);
  return atoi(value);
}

GColor data_get_temp_color(int temp) {
  if (s_min_temp == INIT_MIN_TEMP || s_max_temp == INIT_MAX_TEMP) {
    char *temp_arr = data_get_temp_arr();

    // No data yet
    if (strlen(temp_arr) == 0) return GColorClear;

    // Find values from array
    for (int i = 0; i < 24; i++) {
      const int temp = data_get_strarr_value(temp_arr, i);
      if (temp < s_min_temp) s_min_temp = temp;
      if (temp > s_max_temp) s_max_temp = temp;
    }
  }

  // Lowest/highest
  if (temp == s_min_temp) return GColorBlue;
  if (temp == s_max_temp) return GColorWindsorTan;

  // Near
  if (temp < s_min_temp + 2) return GColorBlueMoon;
  if (temp > s_max_temp - 2) return GColorOrange;

  // Not so near
  if (temp < s_min_temp + 3) return GColorVividCerulean;
  if (temp > s_max_temp - 3) return GColorChromeYellow;

  return GColorClear;
}
