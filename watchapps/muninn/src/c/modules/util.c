#include "util.h"

TextLayer* util_make_text_layer(GRect frame, GFont font) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_font(this, font);
  text_layer_set_background_color(this, GColorClear);
  return this;
}

void util_fmt_time(int timestamp_s, char* buf, int buf_size) {
  if (timestamp_s == DATA_EMPTY) {
    snprintf(buf, buf_size, "-");
    return;
  }

  const time_t t = (time_t)timestamp_s;
  const struct tm *tm_info = gmtime(&t);
  const int hours = tm_info->tm_hour;
  const int mins = tm_info->tm_min;

  snprintf(buf, buf_size, "%02d:%02d %s", hours, mins, hours < 12 ? "AM" : "PM");
}

void util_fmt_time_ago(int then, char *buf, int buf_size) {
  const time_t now = time(NULL);
  int diff_s = now - then;

  // Nudge: always a positive amount of time for display purposes
  if (diff_s < 0) {
    diff_s *= -1;
  }

  int value = 0;
  const char *unit;
  if (diff_s < 60) {
    value = diff_s;
    unit = "s";
  } else if (diff_s < 3600) {
    value = diff_s / 60;
    unit = "m";
  } else if (diff_s < SECONDS_PER_DAY) {
    value = diff_s / 3600;
    unit = "h";
  } else {
    value = diff_s / SECONDS_PER_DAY;
    unit = "d";
  }

  snprintf(buf, buf_size, "%d %s", value, unit);
}
