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

  // Defaults - if unset or newly added
  if (strlen(s_persist_data.temp_unit) == 0) {
    snprintf(s_persist_data.temp_unit, sizeof(s_persist_data.temp_unit), "%s", TEMP_UNIT_C);
  }
  if (strlen(s_persist_data.theme) == 0) {
    snprintf(s_persist_data.theme, sizeof(s_persist_data.theme), "%s", THEME_BLUE);
  }
  if (strlen(s_persist_data.slot_1) == 0) {
    snprintf(s_persist_data.slot_1, sizeof(s_persist_data.slot_1), "%s", COMPLICATION_NONE);
  }
  if (strlen(s_persist_data.slot_2) == 0) {
    snprintf(s_persist_data.slot_2, sizeof(s_persist_data.slot_2), "%s", COMPLICATION_STEP_COUNT);
  }
  if (strlen(s_persist_data.slot_3) == 0) {
    snprintf(s_persist_data.slot_3, sizeof(s_persist_data.slot_3), "%s", COMPLICATION_NONE);
  }
  if (strlen(s_persist_data.slot_4) == 0) {
    snprintf(s_persist_data.slot_4, sizeof(s_persist_data.slot_4), "%s", COMPLICATION_BATTERY);
  }
  if (strlen(s_persist_data.step_goal) == 0) {
    snprintf(s_persist_data.step_goal, sizeof(s_persist_data.step_goal), "%s", STEP_GOAL_10000);
  }

  // TEST
#ifdef TEST
  snprintf(s_persist_data.theme, sizeof(s_persist_data.theme), "%s", THEME_RED);
  snprintf(s_persist_data.slot_1, sizeof(s_persist_data.slot_1), "%s", COMPLICATION_NONE);
  snprintf(s_persist_data.slot_2, sizeof(s_persist_data.slot_2), "%s", COMPLICATION_STEP_COUNT);
  snprintf(s_persist_data.slot_3, sizeof(s_persist_data.slot_3), "%s", COMPLICATION_NONE);
  snprintf(s_persist_data.slot_4, sizeof(s_persist_data.slot_4), "%s", COMPLICATION_BATTERY);
  snprintf(s_persist_data.step_goal, sizeof(s_persist_data.step_goal), "%s", STEP_GOAL_10000);
#endif
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
#ifdef TEST
  return RESOURCE_ID_ICON_STORM;
#endif

  switch (s_app_state.current_code) {
    case 0:
    case 1: // Mainly clear
      return RESOURCE_ID_ICON_CLEAR;
    case 2: // Partly cloudy
    case 3: // Overcast
      return RESOURCE_ID_ICON_CLOUD;
    case 45: // Fog
    case 48: // Depositing rime fog
      return RESOURCE_ID_ICON_FOG;
    case 51: // Light Drizzle
    case 53: // Moderate Drizzle
    case 55: // Dense Drizzle
    case 56: // Light Freezing Drizzle
    case 57: // Dense Freezing Drizzle
    case 61: // Slight Rain
    case 66: // Light Freezing Rain
    case 80: // Slight Rain showers
      return RESOURCE_ID_ICON_LIGHT_RAIN;
    case 63: // Moderate Rain
    case 65: // Heavy Rain
    case 67: // Heavy Freezing Rain
    case 81: // Moderate Rain showers
    case 82: // Violent Rain showers
      return RESOURCE_ID_ICON_HEAVY_RAIN;
    case 85: // Slight Snow showers
    case 86: // Heavy Snow showers
    case 71: // Slight Snow fall
    case 73: // Moderate Snow fall
    case 75: // Heavy Snow fall
    case 77: // Snow grains
      return RESOURCE_ID_ICON_SNOW;
    case 95: // Thunderstorm: Slight or moderate
    case 96: // Thunderstorm with slight hail
    case 97: // Heavy thunderstorm
    case 99: // Thunderstorm with heavy hail
      return RESOURCE_ID_ICON_STORM;
    default:
      return RESOURCE_ID_ICON_BLANK;
  }
};

GColor data_get_theme_day_color() {
  if (strcmp(s_persist_data.theme, THEME_BLUE) == 0) {
    return GColorBlue;
  } else if (strcmp(s_persist_data.theme, THEME_RED) == 0) {
    return GColorRed;
  } else if (strcmp(s_persist_data.theme, THEME_GREEN) == 0) {
    return GColorIslamicGreen;
  } else if (strcmp(s_persist_data.theme, THEME_AQUA) == 0) {
    return GColorTiffanyBlue;
  } else if (strcmp(s_persist_data.theme, THEME_PURPLE) == 0) {
    return GColorVividViolet;
  } else if (strcmp(s_persist_data.theme, THEME_ORANGE) == 0) {
    return GColorOrange;
  } else if (strcmp(s_persist_data.theme, THEME_GRAY) == 0) {
    return GColorDarkGray;
  }

  // Unknown
  return GColorWhite;
}

GColor data_get_theme_night_color() {
  if (strcmp(s_persist_data.theme, THEME_BLUE) == 0) {
    return GColorOxfordBlue;
  } else if (strcmp(s_persist_data.theme, THEME_RED) == 0) {
    return GColorBulgarianRose;
  } else if (strcmp(s_persist_data.theme, THEME_GREEN) == 0) {
    return GColorDarkGreen;
  } else if (strcmp(s_persist_data.theme, THEME_AQUA) == 0) {
    return GColorMidnightGreen;
  } else if (strcmp(s_persist_data.theme, THEME_PURPLE) == 0) {
    return GColorImperialPurple;
  } else if (strcmp(s_persist_data.theme, THEME_ORANGE) == 0) {
    return GColorWindsorTan;
  } else if (strcmp(s_persist_data.theme, THEME_GRAY) == 0) {
    return GColorBlack;
  }

  // Unknown
  return GColorWhite;
}

bool data_is_health_used() {
  return strcmp(s_persist_data.slot_1, COMPLICATION_STEP_COUNT) == 0 ||
    strcmp(s_persist_data.slot_2, COMPLICATION_STEP_COUNT) == 0 ||
    strcmp(s_persist_data.slot_3, COMPLICATION_STEP_COUNT) == 0 ||
    strcmp(s_persist_data.slot_4, COMPLICATION_STEP_COUNT) == 0;
}
