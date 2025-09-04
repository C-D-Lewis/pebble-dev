#pragma once

#include <pebble.h>

// Comment before release!
// #define TEST

#ifdef PBL_PLATFORM_EMERY
  #define DISPLAY_W 220
  #define DISPLAY_H 228
  #define PROJECTION_OFFSET GPoint((DISPLAY_W / 4) - 12, (DISPLAY_H / 2) - 18)
  #define B_W 11
  #define B_H 13
  #define MINUTE_ROW_OFFSET -6
  #define X_NUDGE 0
#else
  #define DISPLAY_W 144
  #define DISPLAY_H 168
  #define PROJECTION_OFFSET GPoint((DISPLAY_W / 4) - 5, (DISPLAY_H / 2) - 12)
  #define B_W 8
  #define B_H 10
  #define MINUTE_ROW_OFFSET -4
  #define X_NUDGE 1 // Make dithering look consistent
#endif

#define ORIGIN_X 0
#define ORIGIN_Y 0
#define HOUR_TENS_ORIGIN GPoint(ORIGIN_X, ORIGIN_Y)
#define HOUR_UNITS_ORIGIN GPoint(ORIGIN_X + X_NUDGE, ORIGIN_Y - (5 * B_W))
#define MINUTE_TENS_ORIGIN GPoint(ORIGIN_X + (8 * B_W), ORIGIN_Y + (6 * B_W) + MINUTE_ROW_OFFSET)
#define MINUTE_UNITS_ORIGIN GPoint(ORIGIN_X + (8 * B_W), B_W + MINUTE_ROW_OFFSET)
#define BOX_SIZE GSize(B_W, B_W)

// Testing purposes
#define T_HOURS 18
#define T_MINUTES 24
#define T_IS_DAY true
#define T_IS_CONNECTED false
