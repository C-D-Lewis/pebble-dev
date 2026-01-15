#include "data.h"

typedef struct {
  bool battery_and_bluetooth;
  bool weather_status;
  bool second_tick;

  GColor color_background;
  GColor color_brackets;
  GColor color_datetime;
  GColor color_complications;
} AppConfig;

typedef enum {
  SK_AppConfig = 0
} StorageKeys;

static AppConfig s_app_config;
static char s_weather_buffer[16];

static void save_all() {
  status_t result = persist_write_data(SK_AppConfig, &s_app_config, sizeof(AppConfig));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing app config: %d", (int)result);
  }
}

void data_init() {
  if (persist_exists(SK_AppConfig)) {
    // Load existing data
    status_t result = persist_read_data(SK_AppConfig, &s_app_config, sizeof(AppConfig));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading app config: %d", (int)result);
    }
  } else {
    // Defaults
    s_app_config.battery_and_bluetooth = true;
    s_app_config.second_tick = true;
    s_app_config.weather_status = true;
    s_app_config.color_background = GColorWhite;
    s_app_config.color_brackets = GColorBlack;
    s_app_config.color_datetime = GColorBlack;
    s_app_config.color_complications = GColorBlack;
    
    save_all();
  }
}

void data_deinit() {
  save_all();
}

bool data_get_boolean(uint32_t key) {
  // Can't use switch with MESSAGE_KEY_ constants directly
  if (key == MESSAGE_KEY_BatteryAndBluetooth) {
    return s_app_config.battery_and_bluetooth;
  } else if (key == MESSAGE_KEY_WeatherStatus) {
    return s_app_config.weather_status;
  } else if (key == MESSAGE_KEY_SecondTick) {
    return s_app_config.second_tick;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key: %d", (int)key);
    return false;
  }
}

void data_set_boolean(uint32_t key, bool value) {
  if (key == MESSAGE_KEY_BatteryAndBluetooth) {
    s_app_config.battery_and_bluetooth = value;
  } else if (key == MESSAGE_KEY_WeatherStatus) {
    s_app_config.weather_status = value;
  } else if (key == MESSAGE_KEY_SecondTick) {
    s_app_config.second_tick = value;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key: %d", (int)key);
  }
}

void data_set_weather_string(char *str) {
  snprintf(s_weather_buffer, sizeof(s_weather_buffer), "%s", str);
}

char* data_get_weather_string() {
  return &s_weather_buffer[0];
  // return "8   RAIN";
}

void data_set_color(uint32_t key, GColor color) {
  if (key == MESSAGE_KEY_ColorBackground) {
    s_app_config.color_background = color;
  } else if (key == MESSAGE_KEY_ColorBrackets) {
    s_app_config.color_brackets = color;
  } else if (key == MESSAGE_KEY_ColorDateTime) {
    s_app_config.color_datetime = color;
  } else if (key == MESSAGE_KEY_ColorComplications) {
    s_app_config.color_complications = color;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key: %d", (int)key);
  }
}

GColor data_get_color(uint32_t key) {
  if (key == MESSAGE_KEY_ColorBackground) {
    return s_app_config.color_background;
    // return GColorBlack;
  } else if (key == MESSAGE_KEY_ColorBrackets) {
    return s_app_config.color_brackets;
    // return GColorDarkGray;
  } else if (key == MESSAGE_KEY_ColorDateTime) {
    return s_app_config.color_datetime;
    // return GColorRed;
  } else if (key == MESSAGE_KEY_ColorComplications) {
    return s_app_config.color_complications;
    // return GColorWhite;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key: %d", (int)key);
    return GColorBlack;
  }
}