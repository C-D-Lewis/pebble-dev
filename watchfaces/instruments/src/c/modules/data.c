#include "data.h"

static AppState s_app_state;
static PersistData s_persist_data;

void data_init() {
  // Default state always
  s_app_state.current_temp = DATA_EMPTY;
  s_app_state.current_code = DATA_EMPTY;

  // Read existing data if it exists
  if (persist_exists(SK_PersistData)) {
    persist_read_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
  }

  // If unset or newly added
  if (strlen(s_persist_data.temp_unit) == 0) {
    snprintf(s_persist_data.temp_unit, sizeof(s_persist_data.temp_unit), "%s", TEMP_UNIT_C);
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

/*********************************** Methods ***********************************/

uint32_t data_get_weather_icon_id() {
  switch (s_app_state.current_code) {
    case 0:
    case 1: // Mainly clear
      return RESOURCE_ID_WEATHER_CLEAR;
    case 3: // Overcast
      return RESOURCE_ID_WEATHER_OVERCAST;
    case 2: // Partly cloudy
    case 45: // Fog
    case 48: // Depositing rime fog
      return RESOURCE_ID_WEATHER_FOG;
    case 51: // Light Drizzle
    case 53: // Moderate Drizzle
    case 55: // Dense Drizzle
    case 56: // Light Freezing Drizzle
    case 57: // Dense Freezing Drizzle
    case 61: // Slight Rain
    case 66: // Light Freezing Rain
    case 80: // Slight Rain showers
      return RESOURCE_ID_WEATHER_LIGHT_RAIN;
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 67: // Heavy Freezing Rain
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
      return RESOURCE_ID_WEATHER_HEAVY_RAIN;
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
      return RESOURCE_ID_WEATHER_SNOW;
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 99: // Thunderstorm with heavy hail
      return RESOURCE_ID_WEATHER_THUNDERSTORM;
    default:
      return RESOURCE_ID_BLANK;
  }
};
