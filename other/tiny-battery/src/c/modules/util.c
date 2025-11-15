#include "util.h"

TextLayer* util_make_text_layer(GRect frame, GFont font) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_font(this, font);
  text_layer_set_background_color(this, GColorClear);
  return this;
}

void util_fmt_time(int timestamp_s, char* buf, int buf_size) {
  const time_t t = (time_t)timestamp_s;
  const struct tm *tm_info = gmtime(&t);
  const int hours = tm_info->tm_hour;
  const int mins = tm_info->tm_min;

  snprintf(buf, buf_size, "%02d:%02d %s", hours, mins, hours < 12 ? "AM" : "PM");
}

void util_fmt_time_ago(int then, char *buf, int buf_size) {
  const time_t t = (time_t)then;
  const time_t now = time(NULL);
  // const struct tm *tm_info = gmtime(&t);
  const int diff_s = now - then;

  const int value = (diff_s < SECONDS_PER_DAY) ? (diff_s / 3600) : (diff_s / (SECONDS_PER_DAY));
  const char *unit = diff_s < SECONDS_PER_DAY ? "hour" : "day";
  snprintf(buf, buf_size, "%d %s%s ago", value, unit, value == 1 ? "" : "s");
}
