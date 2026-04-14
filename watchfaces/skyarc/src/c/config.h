#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#pragma once

// Disable before release!
// #define TEST

#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_CHALK)
  #define OUTER_RING_W 2
  #define OUTER_RING_INSET 12
  #define TEMP_RING_W 8
  #define INNER_RING_W 2
  #define SEP_H 4
  #define DOT_S 2
#else
  #define OUTER_RING_W 3
  #define OUTER_RING_INSET 18
  #define TEMP_RING_W 10
  #define INNER_RING_W 3
  #define SEP_H 5
  #define DOT_S 3
#endif

#define OUTER_SEP_W 2
#define TEMP_RING_INSET (OUTER_RING_INSET + OUTER_RING_W - 3)
#define INNER_RING_INSET (TEMP_RING_INSET + TEMP_RING_W + 1)
#define NOTCH_L scl_x(80)

#define STR_ARR_SIZE 49
#define INIT_MIN_TEMP 50
#define INIT_MAX_TEMP -50
#define SIGNED_OFFSET 50
#define WEATHER_ERROR 1000
#define MIN_WEATHER_INTERVAL_S (60 * 60 - 5)

#define DATA_EMPTY -1000
