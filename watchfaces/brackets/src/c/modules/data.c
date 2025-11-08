#include "data.h"

static bool
  s_battery_and_bluetooth = true,
  s_weather_status = true,
  s_second_tick = true;
static char s_weather_buffer[16];

void data_init() {
  // Nuke versions when keys could have changed
  const int nuked_v_2_0 = 453786;
  if (!persist_exists(nuked_v_2_0)) {
    const int persist_max = 32;
    for(int i = 0; i < persist_max; i++) {
      persist_delete(i);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Nuke complete");
    persist_write_bool(nuked_v_2_0, true);
  }

  if (persist_exists(MESSAGE_KEY_BatteryAndBluetooth)) {
    // Load existing data
    s_battery_and_bluetooth = persist_read_bool(MESSAGE_KEY_BatteryAndBluetooth);
    s_second_tick = persist_read_bool(MESSAGE_KEY_SecondTick);
    s_weather_status = persist_read_bool(MESSAGE_KEY_WeatherStatus);
  } else {
    // Save defaults
    s_battery_and_bluetooth = true;
    s_second_tick = true;
    s_weather_status = true;
    persist_write_bool(MESSAGE_KEY_BatteryAndBluetooth, s_battery_and_bluetooth);
    persist_write_bool(MESSAGE_KEY_SecondTick, s_second_tick);
    persist_write_bool(MESSAGE_KEY_WeatherStatus, s_weather_status);
  }
}

void data_deinit() { }

bool data_get_boolean(uint32_t key) {
  // Can't use switch with MESSAGE_KEY_ constants directly
  if (key == MESSAGE_KEY_BatteryAndBluetooth) {
    return s_battery_and_bluetooth;
  } else if (key == MESSAGE_KEY_WeatherStatus) {
    return s_weather_status;
  } else if (key == MESSAGE_KEY_SecondTick) {
    return s_second_tick;
  } else {
    return false; // Unknown key
  }
}

void data_set_boolean(uint32_t key, bool value) {
  if (key == MESSAGE_KEY_BatteryAndBluetooth) {
    s_battery_and_bluetooth = value;
    persist_write_bool(MESSAGE_KEY_BatteryAndBluetooth, value);
  } else if (key == MESSAGE_KEY_WeatherStatus) {
    s_weather_status = value;
    persist_write_bool(MESSAGE_KEY_WeatherStatus, value);
  } else if (key == MESSAGE_KEY_SecondTick) {
    s_second_tick = value;
    persist_write_bool(MESSAGE_KEY_SecondTick, value);
  }
}

void data_set_weather_string(char *str) {
  snprintf(s_weather_buffer, sizeof(s_weather_buffer), "%s", str);
}

char* data_get_weather_string() {
  return &s_weather_buffer[0];
}
