#include "util.h"

TextLayer* util_make_text_layer(GRect frame, GFont font) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_font(this, font);
  text_layer_set_background_color(this, GColorClear);
  return this;
}

void util_fmt_time(int timestamp_s, char* buf, int buf_size) {
  time_t t = (time_t)timestamp_s;
  struct tm *tm_info = gmtime(&t);
  int hours = 0;
  int mins = 0;
  if (tm_info) {
    hours = tm_info->tm_hour;
    mins = tm_info->tm_min;
  }

  snprintf(buf, buf_size, "%02d:%02d", hours, mins);
}
