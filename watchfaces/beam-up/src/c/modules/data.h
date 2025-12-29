#pragma once

#include <pebble.h>

// TODO: Migrate messageKeys
typedef enum {
  // Features
  DataKeyDate = 0,
  DataKeyAnimations,
  DataKeyBTIndicator,
  DataKeyHourlyVibration, // These must be linear for getting/setting
  DataKeySleep,
  DataNumBoolKeys,            // 5

  // Colors
  DataKeyForegroundColor = 50,
  DataKeyBackgroundColor = 51
} DataKey;

// When settings need nuking
typedef enum {
  V_3_0 = 100,
  V_3_1 = 101
} VersionKeys;

// Methods
void data_init();
void data_deinit();

// Getters/setters
bool data_get_boolean_setting(int data_key);
void data_set_boolean_setting(int data_key, bool value);
GColor data_get_foreground_color();
GColor data_get_background_color();
void data_set_foreground_color(GColor color);
void data_set_background_color(GColor color);
