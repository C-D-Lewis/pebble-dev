#include "data.h"

#if defined(PBL_COLOR)
static GColor s_colors[ColorMax];
#endif

static bool s_animations, s_bluetooth_alert;

void data_init() {
#if defined(PBL_COLOR)
  if(persist_exists(ColorBackground)) {
    // Load the data
    for(int i = 0; i < ColorMax; i++) {
      s_colors[i] = (GColor){ .argb = persist_read_int(i) };
    }
  } else {
    // Default colors
    s_colors[ColorBackground] = GColorBlack;
    s_colors[ColorSides] = GColorOrange;
    s_colors[ColorFace] = GColorYellow;
    for(int i = 0; i < ColorMax; i++) {
      persist_write_int(i, s_colors[i].argb);
    }
  }
#endif

  // Other preferences
  if(persist_exists(PreferenceAnimations)) {
    s_animations = persist_read_bool(PreferenceAnimations);
    s_bluetooth_alert = persist_read_bool(PreferenceBluetoothAlert);
  } else {
    // Default settings
    s_animations = true;
    persist_write_bool(PreferenceAnimations, s_animations);
    s_bluetooth_alert = true;
    persist_write_bool(PreferenceBluetoothAlert, s_bluetooth_alert);
  }
}

void data_set_color(Color which, GColor new_color) {
#if defined(PBL_COLOR)
  s_colors[which] = new_color;
  persist_write_int(which, new_color.argb);
#endif
}

GColor data_get_color(Color which) {
#if defined(PBL_COLOR)
  return s_colors[which];
#endif

  // Dummy B&W value, Segment will draw itself as boxes
  return GColorWhite;
}

void data_set_animations(bool value) {
  s_animations = value;
  persist_write_bool(PreferenceAnimations, s_animations);
}

bool data_get_animations() {
#if defined(PBL_ROUND)
  return false; // Crashes on PTR for unknown reasons
#else
  return s_animations;
#endif
}

void data_set_bluetooth_alert(bool value) {
  s_bluetooth_alert = value;
  persist_write_bool(PreferenceBluetoothAlert, s_bluetooth_alert);
}

bool data_get_bluetooth_alert() {
  return s_bluetooth_alert;
}
