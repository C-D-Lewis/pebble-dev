#pragma once

#include <pebble.h>

#include "data.h"

TextLayer* util_make_text_layer(GRect frame, GFont font);

void util_fmt_time(int timestamp_s, char* buff, int size);

void util_fmt_time_ago(time_t then, char *buff, int size);

int util_hours_until_next_interval();

void util_fmt_time_unit(time_t ts, char *buff, int size);

char* util_get_status_string();

uint32_t util_get_battery_resource_id(int charge_percent);

bool util_is_valid(int v);
