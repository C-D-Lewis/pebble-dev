#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#pragma once

// Disable before release!
// #define TEST

// Layout
#define OUTER_RING_W 2
#define OUTER_RING_INSET 28
#define COMPLICATION_W 20
#define HOUR_DOT_RING_W 20
#define DAY_HAND_W 4
#define CENTER_POST_W 4
#define DOT_S 3
#define ICON_SIZE 28

// Behavior
#define MIN_WEATHER_INTERVAL_S (60 * 60 - 5)

// Constants
#define WEATHER_ERROR 1000
#define DATA_EMPTY -1000
#define TEMP_UNIT_C "C"
#define TEMP_UNIT_F "F"
#define COMPLICATION_NONE "NONE"
#define COMPLICATION_BATTERY "BATTERY"
#define COMPLICATION_STEP_COUNT "STEP_COUNT"
#define THEME_BLUE "BLUE"
#define THEME_RED "RED"
#define THEME_GREEN "GREEN"
#define THEME_AQUA "AQUA"
#define THEME_PURPLE "PURPLE"
#define THEME_ORANGE "ORANGE"
#define THEME_GRAY "GRAY"
#define STEP_GOAL_5000 "5000"
#define STEP_GOAL_7500 "7500"
#define STEP_GOAL_10000 "10000"

// Colors
#define COLOR_BATTERY GColorIslamicGreen
#define COLOR_STEP_COUNT GColorBlueMoon
