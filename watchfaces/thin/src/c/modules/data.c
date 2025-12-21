#include "data.h"

#define SK_AppConfig 0

#define SK_MIGRATION_1 100

typedef struct {
  bool enable_date;
  bool enable_day;
  bool enable_bt;
  bool enable_battery;
  bool enable_second_hand;
  bool enable_steps;

  int color_background;
  int color_hour_minutes;
  int color_seconds;
  int color_notches;
  int color_month_day;
  int color_date;
} AppConfig;

// Cache for speed
static AppConfig s_app_config;

static void save_all() {
  status_t result = persist_write_data(SK_AppConfig, &s_app_config, sizeof(AppConfig));
  if (result < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error writing app config: %d", (int)result);
  }
}

void data_init() {
  // When color options were added we needed to start again
  if (!persist_exists(SK_MIGRATION_1)) {
    persist_delete(SK_AppConfig);
    persist_write_int(SK_MIGRATION_1, 1);
  }

  if(!persist_exists(SK_AppConfig)) {
    // Set defaults
    s_app_config.enable_date = true;
    s_app_config.enable_day = true;
    s_app_config.enable_bt = true;
    s_app_config.enable_battery = true;
    s_app_config.enable_second_hand = true;
    s_app_config.enable_steps = true;

    s_app_config.color_background = GColorBlackARGB8;
    s_app_config.color_hour_minutes = GColorLightGrayARGB8;
    s_app_config.color_seconds = GColorDarkCandyAppleRedARGB8;
    s_app_config.color_notches = GColorWhiteARGB8;
    s_app_config.color_month_day = GColorChromeYellowARGB8;
    s_app_config.color_date = GColorWhiteARGB8;

    save_all();
  } else {
    status_t result = persist_read_data(SK_AppConfig, &s_app_config, sizeof(AppConfig));
    if (result < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading app config: %d", (int)result);
    }
  }
}

void data_deinit() {
  save_all();
}

void data_set_enable(uint32_t key, bool b) {
  // Can't use switch with MESSAGE_KEY_ constants directly
  if (key == MESSAGE_KEY_EnableDate) {
    s_app_config.enable_date = b;
  } else if (key == MESSAGE_KEY_EnableDay) {
    s_app_config.enable_day = b;
  } else if (key == MESSAGE_KEY_EnableBT) {
    s_app_config.enable_bt = b;
  } else if (key == MESSAGE_KEY_EnableBattery) {
    s_app_config.enable_battery = b;
  } else if (key == MESSAGE_KEY_EnableSecondHand) {
    s_app_config.enable_second_hand = b;
  } else if (key == MESSAGE_KEY_EnableSteps) {
    s_app_config.enable_steps = b;
  }
}

bool data_get_enable(uint32_t key) {
  if (key == MESSAGE_KEY_EnableDate) {
    return s_app_config.enable_date;
  } else if (key == MESSAGE_KEY_EnableDay) {
    return s_app_config.enable_day;
  } else if (key == MESSAGE_KEY_EnableBT) {
    return s_app_config.enable_bt;
  } else if (key == MESSAGE_KEY_EnableBattery) {
    return s_app_config.enable_battery;
  } else if (key == MESSAGE_KEY_EnableSecondHand) {
    return s_app_config.enable_second_hand;
  } else if (key == MESSAGE_KEY_EnableSteps) {
    return s_app_config.enable_steps;
  }

  return false;
}

void data_set_color(uint32_t key, int color) {
  if (key == MESSAGE_KEY_ColorBackground) {
    s_app_config.color_background = color;
  } else if (key == MESSAGE_KEY_ColorHourMinutes) {
    s_app_config.color_hour_minutes = color;
  } else if (key == MESSAGE_KEY_ColorSeconds) {
    s_app_config.color_seconds = color;
  } else if (key == MESSAGE_KEY_ColorNotches) {
    s_app_config.color_notches = color;
  } else if (key == MESSAGE_KEY_ColorMonthDay) {
    s_app_config.color_month_day = color;
  } else if (key == MESSAGE_KEY_ColorDate) {
    s_app_config.color_date = color;
  }
}

GColor data_get_color(uint32_t key) {
  if (key == MESSAGE_KEY_ColorBackground) {
    return GColorFromHEX(s_app_config.color_background);
  } else if (key == MESSAGE_KEY_ColorHourMinutes) {
    return GColorFromHEX(s_app_config.color_hour_minutes);
  } else if (key == MESSAGE_KEY_ColorSeconds) {
    return GColorFromHEX(s_app_config.color_seconds);
  } else if (key == MESSAGE_KEY_ColorNotches) {
    return GColorFromHEX(s_app_config.color_notches);
  } else if (key == MESSAGE_KEY_ColorMonthDay) {
    return GColorFromHEX(s_app_config.color_month_day);
  } else if (key == MESSAGE_KEY_ColorDate) {
    return GColorFromHEX(s_app_config.color_date);
  }

  return GColorBlack;
}
