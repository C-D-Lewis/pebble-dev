#pragma once

#include <pebble.h>

typedef enum {
  DataKeyDate = 0,
  DataKeyAnimations,
  DataKeyBTIndicator,
  DataKeyHourlyVibration, // These must be linear for getting/setting
  DataKeySleep,
  DataNumBoolKeys,            // 5
  DataKeyForegroundColor = 50,
  DataKeyBackgroundColor = 51
} DataKey;

typedef enum {
  V_3_0 = 100,    // When settings need nuking
  V_3_1 = 101
} VersionKeys;

void data_init();
void data_deinit();

bool data_get_boolean_setting(int data_key);
void data_set_boolean_setting(int data_key, bool value);

GColor data_get_foreground_color();
GColor data_get_background_color();
void data_set_foreground_color(GColor color);
void data_set_background_color(GColor color);
