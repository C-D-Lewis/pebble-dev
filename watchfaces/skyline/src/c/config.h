#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#pragma once

#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_CHALK)
  #define OUTER_RING_W 3
  #define OUTER_RING_INSET 10
  #define TEMP_RING_W 7
  #define INNER_RING_W 2
  #define SEP_H 4
#else
  #define OUTER_RING_W 4
  #define OUTER_RING_INSET 16
  #define TEMP_RING_W 9
  #define INNER_RING_W 3
  #define SEP_H 5
#endif

#define TEMP_RING_INSET (OUTER_RING_INSET + OUTER_RING_W - 3)
#define INNER_RING_INSET (TEMP_RING_INSET + TEMP_RING_W + 1)

#define STR_ARR_SIZE 49
#define INIT_MIN_TEMP 50
#define INIT_MAX_TEMP -50
#define SIGNED_OFFSET 50
#define WEATHER_ERROR 1000

#define NOTCH_L scl_x(80)
