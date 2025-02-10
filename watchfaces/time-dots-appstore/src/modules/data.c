#include "data.h"

static GColor s_colors[ColorTypeCount];
static bool s_show_minutes, s_bt_alert;

void data_init() {
  const int persist_key_first_time = 54786;
  if(!persist_exists(persist_key_first_time)) {
    persist_write_bool(persist_key_first_time, false);

    // Set defaults
    data_set_color(ColorTypeBackground, PBL_IF_COLOR_ELSE(GColorDukeBlue, GColorWhite));
    data_set_color(ColorTypeRing, PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorBlack));
    data_set_color(ColorTypeDotInactive, PBL_IF_COLOR_ELSE(GColorBlack, GColorDarkGray));
    data_set_color(ColorTypeDotActive, GColorWhite);
  } else {
    // Load from persist
    data_set_color(ColorTypeBackground, (GColor) { .argb = persist_read_int(ColorTypeBackground) });
    data_set_color(ColorTypeRing, (GColor) { .argb = persist_read_int(ColorTypeRing) });
    data_set_color(ColorTypeDotInactive, (GColor) { .argb = persist_read_int(ColorTypeDotInactive) });
    data_set_color(ColorTypeDotActive, (GColor) { .argb = persist_read_int(ColorTypeDotActive) });
  }

  const int persist_key_v_1_3 = 432987;
  if(!persist_exists(persist_key_v_1_3)) {
    persist_write_bool(persist_key_v_1_3, false);

    data_set_feature(FeatureKeyShowMinutes, true);
    data_set_feature(FeatureKeyBTAlert, true);
  } else {
    data_set_feature(FeatureKeyShowMinutes, persist_read_bool(FeatureKeyShowMinutes));
    data_set_feature(FeatureKeyBTAlert, persist_read_bool(FeatureKeyBTAlert));
  }
}

void data_deinit() { }

void data_set_color(ColorType type, GColor color) {
  s_colors[type] = color;

#if defined(PBL_COLOR)
  persist_write_int(type, color.argb);
#endif
}

GColor data_get_color(ColorType type) {
#if defined(PBL_COLOR)
  return s_colors[type];
#else
  switch(type) {
    case ColorTypeBackground:  return GColorWhite;
    case ColorTypeRing:        return GColorDarkGray;
    case ColorTypeDotInactive: return GColorBlack;
    case ColorTypeDotActive:   return GColorWhite;
    default: return GColorBlack;
  }
#endif
}

void data_set_feature(FeatureKey key, bool b) {
  switch(key) {
    case FeatureKeyShowMinutes:
      s_show_minutes = b;
      break;
    case FeatureKeyBTAlert:
      s_bt_alert = b;
      break;
    default: break;
  }
  persist_write_bool(key, b);
}

bool data_get_feature(FeatureKey key) {
  switch(key) {
    case FeatureKeyShowMinutes: return s_show_minutes;
    case FeatureKeyBTAlert:     return s_bt_alert;
    default: return false;
  }
}
