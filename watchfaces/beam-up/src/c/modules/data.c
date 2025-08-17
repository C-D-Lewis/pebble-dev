#include "data.h"

// Cache all data
static GColor s_foreground, s_background;
static bool s_bool_settings[DataNumBoolKeys];

static void write_defaults() {
  s_bool_settings[DataKeyDate] = false;
  s_bool_settings[DataKeyAnimations] = true;
  s_bool_settings[DataKeyBTIndicator] = true;
  s_bool_settings[DataKeyHourlyVibration] = false;
  s_bool_settings[DataKeySleep] = false;
  s_foreground = GColorWhite;
  s_background = GColorBlack;
  data_deinit();
}

void data_init() {
  // Nuke previus version settings
  if(!persist_exists(V_3_1)) {
    persist_write_bool(V_3_1, true);

    for(int i = 0; i < DataNumBoolKeys; i++) {
      persist_delete(i);
    }
    persist_delete(DataKeyForegroundColor);
    persist_delete(DataKeyBackgroundColor);

    write_defaults();
  } else {
    // Read settings
    for(int i = 0; i < DataNumBoolKeys; i++) {
      s_bool_settings[i] = persist_read_bool(i);
    }
    s_foreground = (GColor){ .argb = persist_read_int(DataKeyForegroundColor) };
    s_background = (GColor){ .argb = persist_read_int(DataKeyBackgroundColor) };
  }
}

void data_deinit() {
  // Store current values
  for(int i = 0; i < DataNumBoolKeys; i++) {
    persist_write_bool(i, s_bool_settings[i]);
  }
  persist_write_int(DataKeyForegroundColor, s_foreground.argb);
  persist_write_int(DataKeyBackgroundColor, s_background.argb);
}

bool data_get_boolean_setting(int data_key) {
  return (data_key < DataNumBoolKeys && persist_exists(data_key)) ? s_bool_settings[data_key] : false;
}

void data_set_boolean_setting(int data_key, bool value) {
  if(data_key < DataNumBoolKeys) {
    s_bool_settings[data_key] = value;
  }
}

GColor data_get_foreground_color() {
#if defined(PBL_COLOR)
  return s_foreground;
#else
  return GColorWhite;
#endif
}

GColor data_get_background_color() {
#if defined(PBL_COLOR)
  return s_background;
#else
  return GColorBlack;
#endif
}

void data_set_foreground_color(GColor color) {
  s_foreground = color;
}

void data_set_background_color(GColor color) {
  s_background = color;
}
