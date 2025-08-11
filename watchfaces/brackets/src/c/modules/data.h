#pragma once

#include <pebble.h>

void data_init();

void data_deinit();

void data_set_boolean(uint32_t key, bool b);

bool data_get_boolean(uint32_t key);

void data_set_color(uint32_t key, GColor color);

GColor data_get_color(uint32_t key);
