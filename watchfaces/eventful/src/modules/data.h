#pragma once

#include <pebble.h>

void data_init();

void data_set_update_frequency(int mins);

int data_get_update_frequency();
