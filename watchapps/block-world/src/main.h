/*
 * Block builder app for Pebble SDK 3.0
 *
 * Chris Lewis
 */

#pragma once

#include <pebble-isometric/pebble-isometric.h>

// Compile config
// #define BENCHMARK
// #define ROTATION
#define OUTLINES

extern int g_angle;

typedef struct {
  int8_t w;
  int8_t h;
} __attribute__((__packed__)) GSizeSmall;

// Map settings
#ifdef PBL_PLATFORM_EMERY
  #define DISPLAY_W 220
  #define DISPLAY_H 228
  #define STATUS_LAYER_INSETS GEdgeInsets(0, 0, 200, 0)
  #define BLOCK_SIZE 6
  #define PROJECTION_OFFSET GPoint((DISPLAY_W / 2) - 10, (DISPLAY_H / 2) + (DISPLAY_H / 15))
#elif defined(PBL_PLATFORM_CHALK)
  #define DISPLAY_W 180
  #define DISPLAY_H 180
  #define STATUS_LAYER_INSETS GEdgeInsets(30, 0, 130, 0)
  #define BLOCK_SIZE 5
  #define PROJECTION_OFFSET GPoint(DISPLAY_W / 2, (DISPLAY_H / 2) + (DISPLAY_H / 15))
#else
  #define DISPLAY_W 144
  #define DISPLAY_H 168
  #define STATUS_LAYER_INSETS GEdgeInsets(0, 0, 150, 0)
  #define BLOCK_SIZE 5
  #define PROJECTION_OFFSET GPoint(DISPLAY_W / 2, (DISPLAY_H / 2) + (DISPLAY_H / 15))
#endif
#define GRID_WIDTH 14
#define GRID_HEIGHT 14
#define GRID_DEPTH 14
#define FRAME_RATE_IDLE 25
#define SKY_HEIGHT BLOCK_SIZE * GRID_DEPTH

// Behavior
#define MAX_CLOUDS 5
#define CLOUD_SPAWN_CHANCE 95

// Generation
#define SAND_SEED_BIAS 90
#define SAND_CLUMP_BIAS 60
#define WATER_CLUMP_BIAS 75
#define WATER_NEIGHBOURS 9
#define SAND_NEIGHBOURS 5

// Colors
#define COLOR_GRASS GColorJaegerGreen
#define COLOR_SAND GColorPastelYellow
#define COLOR_WATER GColorBlueMoon
#define COLOR_CLOUD GColorWhite
#define COLOR_RED GColorOrange
#define COLOR_STONE GColorLightGray
#define COLOR_INVISIBLE GColorBlack

// UI
#define MODE_X 0
#define MODE_Y 1
#define MODE_Z 2
#define MODE_BLOCK_CHOICE 3

// Blocks
#define BLOCK_GRASS 0
#define BLOCK_SAND 1
#define BLOCK_WATER 2
#define BLOCK_RED 3
#define BLOCK_STONE 4
#define BLOCK_DELETE 5
#define NUM_BLOCK_TYPES 6