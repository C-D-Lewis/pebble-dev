#pragma once

#include <pebble.h>

#include "data.h"

GPoint util_make_hand_point(int quantity, int intervals, int len, GPoint center);

int util_get_time_angle(char *time_str);

int util_convert_temp(int val_c);
