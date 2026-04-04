#pragma once

#include <pebble.h>

#define PROJECTION_OFFSET GPoint((PS_DISP_W / 2), -32)
#if defined(PBL_PLATFORM_BASALT) || defined(PBL_PLATFORM_CHALK)
  #define B_W 28
#else
  #define B_W 34
#endif

#define B_H 16

#define BOX_SIZE GSize(B_W, B_W)
#define GRID_SIZE 12

#define NUM_COLORS 6
#define Z_MOD 16
