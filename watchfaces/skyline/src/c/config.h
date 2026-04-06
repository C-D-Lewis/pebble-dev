#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#pragma once

#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_CHALK)
  #define OUTER_RING_W 3
  #define OUTER_RING_INSET 10
  #define TEMP_RING_W 6
  #define SEP_H 4
#else
  #define OUTER_RING_W 4
  #define OUTER_RING_INSET 16
  #define TEMP_RING_W 8
  #define SEP_H 5
#endif

#define TEMP_RING_INSET (OUTER_RING_INSET + OUTER_RING_W - 3)
#define INNER_RING_INSET (TEMP_RING_INSET + TEMP_RING_W + 2)