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
    return "Awaiting discharges...";
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

void util_draw_braid(GContext *ctx, GRect rect) {
#if !defined(PBL_PLATFORM_APLITE)
  graphics_draw_bitmap_in_rect(ctx, bitmaps_get(RESOURCE_ID_BRAID), rect);
#else
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, rect, 0, GCornerNone);

  // Draw braid without image on Aplite to save memory
  const int size = 2;
  const int start_x = size;
  const int end_x = rect.size.w - size;
  const int start_y = rect.origin.y + size;
  const int end_y = rect.origin.y + rect.size.h - size;
  for (int y = start_y; y < end_y; y += size) {
    const bool offset = ((y - start_y) / size) % 2 == 1;
    for (int x = start_x + (offset ? size : 0); x < end_x; x += (2 * size)) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(
        ctx,
        GRect(x, y, size, size),
        0,
        GCornerNone
      );
    }
  }
#endif
}
