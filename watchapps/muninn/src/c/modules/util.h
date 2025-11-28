#pragma once

#include <pebble.h>

#include "data.h"

TextLayer* util_make_text_layer(GRect frame, GFont font);

void util_fmt_time(int timestamp_s, char* buf, int buf_size);

void util_fmt_time_ago(int then, char *buf, int buf_size);

int util_hours_until_next_interval();
