#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#pragma once

// Disable before release!
// #define TEST

// Layout
#define OUTER_RING_W 2
#define OUTER_RING_INSET 26
#define TEMP_RING_W 10
#define DAY_HAND_W 4
#define CENTER_POST_W 4
#define DOT_S 3
#define ICON_SIZE 28

// Config
#define MIN_WEATHER_INTERVAL_S (60 * 60 - 5)

// Constants
#define WEATHER_ERROR 1000
#define DATA_EMPTY -1000
#define TEMP_UNIT_C "C"
#define TEMP_UNIT_F "F"
