#include "data.h"

#define STR_ARR_SIZE (24 * 2)

static int s_current_temp, s_current_code;
static char s_temp_arr[STR_ARR_SIZE];
static char s_precip_arr[STR_ARR_SIZE];
static char s_code_arr[STR_ARR_SIZE];

/***************************** Getters / setters ******************************/

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

/********************************** Methods ***********************************/

GColor data_get_weather_color(int code) {
  switch (code) {
    case 0:
    case 1: // Mainly clear
      return GColorClear;
    case 2: // Partly cloudy
    case 3: // Overcast
    // Fog and Rime Fog (45, 48)
    case 45: // Fog
    case 48: // Depositing rime fog
      return GColorDarkGray;
    // Drizzle (51, 53, 55) & Freezing Drizzle (56, 57)
    case 51: // Light Drizzle
    case 53: // Moderate Drizzle
    case 55: // Dense Drizzle
    case 56: // Light Freezing Drizzle
    case 57: // Dense Freezing Drizzle
    // Rain (61, 63, 65) & Freezing Rain (66, 67)
    case 61: // Slight Rain
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 66: // Light Freezing Rain
    case 67: // Heavy Freezing Rain
    // Rain Showers (80, 81, 82)
    case 80: // Slight Rain showers
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
      return GColorBlueMoon;
    // Snow Fall and Snow Grains (71, 73, 75, 77)
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
      return GColorWhite;
    // Thunderstorm (95, 96, 99)
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return GColorYellow;
    // Default/Unknown Code
    default:
      return GColorClear;
  }
};

/**
 * Arrays are two chars per item, 24 items.
 *
 * {
 *   "CURRENT_TEMP": 12,
 *   "CURRENT_CODE": 2,
 *   "TEMPS": "121211101009080910101112121212121211100807060606",
 *   "PRECIP": "000000000000000000020713161408040201000000000000",
 *   "CODES": "030303030303030302030202030202030302000100000000"
 * }
 */
int data_get_hour_weather_code(int hour) {
  char *code_arr = data_get_code_arr();

  // Get the code for the current hour
  const int code_index = hour * 2; // Two chars per code
  char code_str[3] = {code_arr[code_index], code_arr[code_index + 1], '\0'};
  return atoi(code_str);
}
