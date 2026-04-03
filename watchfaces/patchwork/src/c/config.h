#pragma once

#include <pebble.h>

// Comment before release!
// #define TEST

#define PROJECTION_OFFSET GPoint((PS_DISP_W / 2), -24)
#define B_W 34
#define B_H 12

#define ORIGIN_X 0
#define ORIGIN_Y 0
#define BOX_SIZE GSize(B_W, B_W)
#define GRID_SIZE 10

// Testing purposes
#define T_HOURS 18
#define T_MINUTES 24
#define T_IS_DAY true
#define T_IS_CONNECTED false

#define NUM_COLORS 8
#define Z_MOD 10