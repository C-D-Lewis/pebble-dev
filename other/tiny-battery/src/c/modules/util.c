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
  const int diff_s = now - then;

  int value = (diff_s < SECONDS_PER_DAY) ? (diff_s / 3600) : (diff_s / (SECONDS_PER_DAY));
  APP_LOG(APP_LOG_LEVEL_INFO, "%d %d %d %d", (int)t, (int)now, diff_s, value);
  const char *unit = diff_s < SECONDS_PER_DAY ? "h" : "d";

  // Nudge: always a positive amount of time for display purposes
  if (value < 0) {
    value *= -1;
  }

  snprintf(buf, buf_size, "%d %s", value, unit);
}
