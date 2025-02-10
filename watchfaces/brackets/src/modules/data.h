#pragma once

#include <pebble.h>

#define DATA_NUM_BOOLEANS 4
#define DATA_NUM_COLORS   6

typedef enum {
  AppKeyBatteryMeter = 0,
  AppKeyBluetoothAlert,
  AppKeyDashedLine,
  AppKeySecondTick,

  AppKeyBackgroundColor,
  AppKeyDateColor,
  AppKeyTimeColor,
  AppKeyBracketColor,
  AppKeyLineColor,
  AppKeyComplicationColor,
  
  AppKeyMax
} AppKey;

void data_init();

void data_deinit();

void data_set_boolean(int key, bool b);

bool data_get_boolean(int key);

void data_set_color(int key, GColor color);

GColor data_get_color(int key);
