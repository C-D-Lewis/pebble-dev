#pragma once

#include <pebble.h>

void data_init();

void data_deinit();

char* data_get_palette();

void data_set_palette(char* palette);
