#pragma once

#include <pebble.h>

#include "data.h"

GPoint util_make_hand_point(int quantity, int intervals, int len, GPoint center);

int util_convert_temp(PersistData *persist_data, int val_c);

int util_convert_wind_speed(PersistData *persist_data, int val_kph);

bool util_weather_data_is_valid();
