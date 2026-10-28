#include "util.h"

GPoint util_make_hand_point(int quantity, int intervals, int len, GPoint center) {
  const int angle = (TRIG_MAX_ANGLE * quantity) / intervals;
  return (GPoint) {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)len / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)len / TRIG_MAX_RATIO) + center.y,
  };
}

int util_get_time_angle(char *time_str) {
  const char h_arr[3] = {time_str[0], time_str[1], '\0'};
  const char m_arr[3] = {time_str[3], time_str[4], '\0'};
  const int hour = atoi(h_arr);
  const int mins = atoi(m_arr);
  const int angle = ((hour * 60) + mins) * TRIG_MAX_ANGLE / MINUTES_PER_DAY;
  return angle;
}

int util_convert_temp(int val_c) {
  PersistData *persist_data = data_get_persist_data();
  return strcmp(persist_data->temp_unit, "F") == 0
    ? (val_c * 9 / 5) + 32
    : val_c;
}
