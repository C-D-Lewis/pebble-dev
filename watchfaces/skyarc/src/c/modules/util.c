#include "util.h"

GPoint util_make_hand_point(int quantity, int intervals, int len, GPoint center) {
  const int angle = (TRIG_MAX_ANGLE * quantity) / intervals;
  return (GPoint) {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)len / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)len / TRIG_MAX_RATIO) + center.y,
  };
}

int util_convert_temp(PersistData *persist_data, int val_c) {
  return strcmp(persist_data->temp_unit, "F") == 0
    ? (val_c * 9 / 5) + 32
    : val_c;
}

int util_convert_wind_speed(PersistData *persist_data, int val_kph) {
  return strcmp(persist_data->wind_unit, WIND_UNIT_MPH) == 0
    ? (val_kph * 1000) / 1609
    : val_kph;
}
