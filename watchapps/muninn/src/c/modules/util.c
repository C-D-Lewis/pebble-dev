#include "util.h"

TextLayer* util_make_text_layer(GRect frame, GFont font) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_font(this, font);
  text_layer_set_background_color(this, GColorClear);
  return this;
}

void util_fmt_time(int timestamp_s, char* buff, int size) {
  if (timestamp_s < 0) {
    snprintf(buff, size, "-");
    return;
  }

  const time_t t = (time_t)timestamp_s;
  const struct tm *tm_info = localtime(&t);
  const int hours = tm_info->tm_hour;
  const int mins = tm_info->tm_min;

  snprintf(buff, size, "%02d:%02d", hours, mins);
}

void util_fmt_time_ago(time_t then, char *buff, int size) {
  int diff_s = time(NULL) - then;

  // Fudge: always a positive amount of time for display purposes
  diff_s *= diff_s < 0 ? -1 : 1;

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

  snprintf(buff, size, "%d%s", value, unit);
}

int util_hours_until_next_interval() {
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);

  int hour = tm_info->tm_hour;
  int rem = hour % WAKEUP_MOD_H;
  int hours = WAKEUP_MOD_H - rem;
  if (hours == 0) hours = WAKEUP_MOD_H;
  return hours;
}

void util_fmt_time_unit(time_t ts, char *buff, int size) {
  int value = 0;
  const char *unit;

  // TODO: DRY this out with util_fmt_time_ago()
  if (ts < 60) {
    value = ts;
    unit = "s";
  } else if (ts < 3600) {
    value = ts / 60;
    unit = "m";
  } else if (ts < SECONDS_PER_DAY) {
    value = ts / 3600;
    unit = "h";
  } else {
    value = ts / SECONDS_PER_DAY;
    unit = "d";
  }

  snprintf(buff, size, "%d%s", value, unit);
}

char* util_get_status_string() {
  const bool is_enabled = util_is_not_status(data_get_wakeup_id());
  if (!is_enabled) return "Not monitoring";

  // No readings at all yet
  if (!util_is_not_status(data_get_last_sample_time())) return "Awaiting sample...";

  // Edge case here: no change or charging samples don't count
  // We can't produce a prediction AT ALL unless we have 'discharging' samples...
  if (data_get_valid_samples_count() < MIN_SAMPLES) {
    static char s_buff[32];
    snprintf(s_buff, sizeof(s_buff), "Awaiting discharge...");
    return &s_buff[0];
  }

  // Ongoing readings
  return "Passively monitoring";
}

uint32_t util_get_battery_resource_id(int charge_percent) {
  if (charge_percent > 66) return RESOURCE_ID_BATTERY_HIGH;
  if (charge_percent > 33) return RESOURCE_ID_BATTERY_MEDIUM;
  return RESOURCE_ID_BATTERY_LOW;
}

bool util_is_not_status(int v) {
  return v != STATUS_EMPTY && v != STATUS_CHARGED && v != STATUS_NO_CHANGE;
}
