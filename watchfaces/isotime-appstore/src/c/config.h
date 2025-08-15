#pragma once

#include <pebble.h>

// #define DRAW_BG
// #define BENCHMARK

#define FRAME_RATE_HIGH PBL_IF_ROUND_ELSE(10, 30)

#define HOURS_OFFSET      40
#define MINS_OFFSET       25

#if defined(PBL_PLATFORM_APLITE) || defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_DIORITE)
#define PROJECTION_OFFSET GPoint(100, 45)
#define BLOCK_SIZE        10
#define BASE_Y            0
#elif defined(PBL_PLATFORM_CHALK)
#define PROJECTION_OFFSET GPoint(120, 45)
#define BLOCK_SIZE        10
#define BASE_Y            0
#elif defined(PBL_PLATFORM_EMERY)
#define PROJECTION_OFFSET GPoint(133, 45)
#define BLOCK_SIZE        13
#define BASE_Y            -10
#endif
