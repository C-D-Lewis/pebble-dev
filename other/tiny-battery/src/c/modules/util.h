#pragma once

#include <pebble.h>

TextLayer* util_make_text_layer(GRect frame, GFont font);

void util_fmt_time(int timestamp_s, char* buf, int buf_size);
