#pragma once

#include <pebble.h>

#include "../config.h"

void data_init();

void data_deinit();

char* data_get_line_name(int type);

char* data_get_line_state(int type);

char* data_get_line_reason(int type);

GColor data_get_line_color(int type);

GColor data_get_line_state_color(int type);

bool data_get_line_color_is_striped(int type);

void data_set_progress(int progress);

int data_get_progress();
