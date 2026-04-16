#include "data.h"

static AppState s_app_state;
static PersistData s_persist_data;

static int s_min_temp = INIT_MIN_TEMP, s_max_temp = INIT_MAX_TEMP;

void data_init() {
  // Default state always
  s_app_state.current_code = DATA_EMPTY;

  // Read existing data if it exists
  if (persist_exists(SK_PersistData)) {
    persist_read_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
  }

  // If unset or newly added
  if (strlen(s_persist_data.wind_unit) == 0) {
    snprintf(s_persist_data.wind_unit, sizeof(s_persist_data.wind_unit), "%s", WIND_UNIT_MPH);
  }
  if (strlen(s_persist_data.temp_unit) == 0) {
    snprintf(s_persist_data.temp_unit, sizeof(s_persist_data.temp_unit), "%s", TEMP_UNIT_C);
  }
  if (strlen(s_persist_data.animations) == 0) {
    snprintf(s_persist_data.animations, sizeof(s_persist_data.animations), "true");
  }
  if (strlen(s_persist_data.color_bg) == 0) {
    snprintf(s_persist_data.color_bg, sizeof(s_persist_data.color_bg), COLOR_BLACK);
  }
  if (s_persist_data.tap_timeout == 0) {
    s_persist_data.tap_timeout = 5;
  }
}

void data_deinit() {
  persist_write_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
}

/***************************** Getters / setters ******************************/

AppState* data_get_app_state() {
  return &s_app_state;
}

PersistData* data_get_persist_data() {
  return &s_persist_data;
}

int data_get_min_temp() {
  return s_min_temp;
}

int data_get_max_temp() {
  return s_max_temp;
}

// Maybe there is a way to convert a Clay color string to a GColor?
GColor data_get_bg_color() {
#ifdef PBL_COLOR
  if (strcmp(s_persist_data.color_bg, COLOR_BLACK) == 0) return GColorBlack;
  if (strcmp(s_persist_data.color_bg, COLOR_OXFORD_BLUE) == 0) return GColorOxfordBlue;
  if (strcmp(s_persist_data.color_bg, COLOR_BULGARIAN_ROSE) == 0) return GColorBulgarianRose;
  if (strcmp(s_persist_data.color_bg, COLOR_DARK_GREEN) == 0) return GColorDarkGreen;
  if (strcmp(s_persist_data.color_bg, COLOR_CHROME_YELLOW) == 0) return GColorChromeYellow;
  return GColorBlack;
#else
  return GColorBlack;
#endif
}

/********************************** Methods ***********************************/

static GColor data_get_weather_color_color(int code) {
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

static GColor data_get_weather_color_bw(int code) {
  switch (code) {
    case 0:
    case 1: // Mainly clear
      return GColorClear;
    case 3: // Overcast
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
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 67: // Heavy Freezing Rain
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return GColorWhite;  // A bit of a cop out, but they're all worth noting
    default:
      return GColorClear;
  }
};

GColor data_get_weather_color(int code) {
#ifdef PBL_COLOR
  return data_get_weather_color_color(code);
#else
  return data_get_weather_color_bw(code);
#endif
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
  // Works, because of the offset operation on zeros
  if (s_min_temp == INIT_MIN_TEMP || s_max_temp == INIT_MAX_TEMP) {
    char *temp_arr = s_app_state.temp_arr;
    // No data yet
    if (strlen(temp_arr) == 0) return GColorClear;

    // Find values from array
    for (int i = 0; i < 24; i++) {
      const int temp = data_get_strarr_value(temp_arr, i) - SIGNED_OFFSET;
      if (temp < s_min_temp) s_min_temp = temp;
      if (temp > s_max_temp) s_max_temp = temp;
    }
  }

  // Lowest/highest
  if (temp == s_min_temp) return PBL_IF_COLOR_ELSE(GColorBlue, GColorWhite);
  if (temp == s_max_temp) return PBL_IF_COLOR_ELSE(GColorRed, GColorWhite);

  // Near
  if (temp < s_min_temp + 2) return PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorLightGray);
  if (temp > s_max_temp - 2) return PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorLightGray);

  // Less near if there's a high enough range
  const int range = s_max_temp - s_min_temp;
  if (range > 6) {
    if (temp < s_min_temp + 3) return PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorLightGray);
    if (temp > s_max_temp - 3) return PBL_IF_COLOR_ELSE(GColorRajah, GColorLightGray);
  }

  // Average or mid-range
  return GColorBlack;
}

char* data_get_weather_str(int code) {
  switch (code) {
    case 0:
    case 1:
      return "Clear";
    case 3:
    case 2:
      return "Cloud";
    case 45:
    case 48:
      return "Fog";
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
    case 61:
    case 66:
    case 80:
      return "Showers";
    case 63:
    case 65:
    case 67:
    case 81:
    case 82:
      return "Rain";
    case 85:
    case 86:
    case 71:
    case 73:
    case 75:
    case 77:
      return "Snow";
    case 95:
    case 96:
    case 99:
      return "Thunder";
    default:
      return "?";
  }
}

GColor data_get_precip_color(int precip_chance) {
#ifdef PBL_COLOR
  if (precip_chance <= 10) return GColorClear;
  if (precip_chance < 33) return GColorVividCerulean;
  if (precip_chance < 66) return GColorBlueMoon;
  if (precip_chance < 90) return GColorBlue;
  return GColorDukeBlue;
#else
  return GColorLightGray;
#endif
};
