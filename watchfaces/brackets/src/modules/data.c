#include "data.h"

static bool s_booleans[DATA_NUM_BOOLEANS];
static GColor s_colors[DATA_NUM_COLORS];

static int color_key(int key) {
  return key - DATA_NUM_BOOLEANS;
}

void data_init() {
  if(persist_exists(AppKeyBatteryMeter)) {
    // Load existing data
    for(int i = 0; i < DATA_NUM_COLORS + DATA_NUM_BOOLEANS; i++) {
      if(i < DATA_NUM_BOOLEANS) {
        s_booleans[i] = persist_read_bool(i);
      } else {
        s_colors[color_key(i)] = (GColor){ .argb = persist_read_int(i) };
      }
    }
  } else {
    // Set defaults
    s_booleans[AppKeyBatteryMeter] = true;
    s_booleans[AppKeyBluetoothAlert] = false;
    s_booleans[AppKeyDashedLine] = true;
    s_booleans[AppKeySecondTick] = true;
    s_colors[color_key(AppKeyBackgroundColor)] = GColorWhite;
    s_colors[color_key(AppKeyDateColor)] = GColorBlack;
    s_colors[color_key(AppKeyTimeColor)] = GColorBlack;
    s_colors[color_key(AppKeyBracketColor)] = GColorBlack;
    s_colors[color_key(AppKeyLineColor)] = GColorDarkGray;
    s_colors[color_key(AppKeyComplicationColor)] = GColorBlack;

    // Save defaults
    for(int i = 0; i < DATA_NUM_COLORS + DATA_NUM_BOOLEANS; i++) {
      if(i < DATA_NUM_BOOLEANS) {
        persist_write_bool(i, s_booleans[i]);
      } else {
        persist_write_int(i, s_colors[color_key(i)].argb);
      }
    }
  }
}

void data_deinit() { }

void data_set_boolean(int key, bool b) {
  s_booleans[key] = b;
  persist_write_bool(key, b);
}

bool data_get_boolean(int key) {
  return s_booleans[key];
}

void data_set_color(int key, GColor color) {
  s_colors[color_key(key)] = color;
  persist_write_int(key, color.argb);
}

GColor data_get_color(int key) {
  return s_colors[color_key(key)];
}
