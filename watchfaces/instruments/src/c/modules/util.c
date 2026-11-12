#include "util.h"

/**
 * Adjust value before integer truncation.
 */
static int round_div(int value, int divisor) {
  return value >= 0
    ? (value + divisor / 2) / divisor
    : (value - divisor / 2) / divisor;
}

GPoint util_make_hand_point(int quantity, int intervals, int len, GPoint center) {
  const int angle = (TRIG_MAX_ANGLE * quantity) / intervals;
  const int x = round_div(sin_lookup(angle) * (int32_t)len, TRIG_MAX_RATIO);
  const int y = round_div(-cos_lookup(angle) * (int32_t)len, TRIG_MAX_RATIO);

  return (GPoint) { .x = center.x + x, .y = center.y + y };
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

int util_get_month_progress() {
  time_t t = time(NULL);
  struct tm now = *localtime(&t);

  // Find start of this month
  struct tm first = now;
  first.tm_mday = 1;
  first.tm_hour = 0;
  first.tm_min = 0;
  first.tm_sec = 0;

  // Find start of next month
  struct tm next = first;
  next.tm_mon++;

  // Get time in seconds
  time_t start = mktime(&first);
  time_t end = mktime(&next);
  const int days_in_month = (int)((end - start) / SECONDS_PER_DAY);
  return (now.tm_mday * 100) / days_in_month;
}

int util_get_year_progress(void) {
  time_t t = time(NULL);
  struct tm now = *localtime(&t);

  // Start of this year
  struct tm first = now;
  first.tm_mon = 0;
  first.tm_mday = 1;
  first.tm_hour = 0;
  first.tm_min = 0;
  first.tm_sec = 0;

  // Start of next year
  struct tm next = first;
  next.tm_year++;

  time_t start = mktime(&first);
  time_t end = mktime(&next);
  const int days_in_year = (int)((end - start) / SECONDS_PER_DAY);
  const int day_of_year = now.tm_yday + 1;
  return (day_of_year * 100 + days_in_year / 2) / days_in_year;
}
