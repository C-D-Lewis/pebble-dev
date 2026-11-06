#include <pebble.h>

#pragma once

// Disable before release!
// #define TEST

// Layout
#define OUTER_RING_W 3
#define OUTER_RING_INSET 30
#define COMPLICATION_W 21
#define HOUR_DOT_RING_W 22
#define DAY_HAND_W 4
#define CENTER_POST_W 6
#define HOUR_DOT_S 4
#define ICON_SIZE 28
#define GAUGE_DOT_S 2

// Behavior
#define MIN_WEATHER_INTERVAL_S (60 * 60 - 5)

// Constants
#define CHARS_PER_ITEM 2
#define NUM_FORECAST_ITEMS 6
#define STR_ARR_SIZE ((CHARS_PER_ITEM * NUM_FORECAST_ITEMS) + 1)
#define DATA_EMPTY -1000
#define DATA_ERROR 1000

// Enums
#define TEMP_UNIT_C "C"
#define TEMP_UNIT_F "F"
#define COMP_NONE "NONE"
#define COMP_BATTERY "BATTERY"
#define COMP_STEP_COUNT "STEP_COUNT"
#define COMP_NEXT_RAIN "NEXT_RAIN"
#define COMP_NEXT_WEATHER "NEXT_WEATHER"
#define COMP_TEMP_RANGE "TEMP_RANGE"
#define COMP_PHONE_BATTERY "PHONE_BATTERY"
#define COMP_MONTH_PROGRESS "MONTH_PROGRESS"
#define COMP_YEAR_PROGRESS "YEAR_PROGRESS"
#define COMP_MOON_PHASE "MOON_PHASE"
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
#define COLOR_PHONE_BATTERY GColorJaegerGreen
#define COLOR_STEP_COUNT GColorJazzberryJam
#define COLOR_COMPLICATION GColorWhite
#define COLOR_ERROR GColorVividViolet
#define COLOR_LOW_TEMP GColorBlueMoon
#define COLOR_HIGH_TEMP GColorRed
#define COLOR_MONTH_PROGRESS GColorLightGray
#define COLOR_YEAR_PROGRESS GColorDarkGray
