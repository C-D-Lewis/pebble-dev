#pragma once

#include <pebble.h>

/////////////////////////////////////
// DO NOT CHANGE WITHOUT MIGRATION //
// Hours between wakeups           //
#define WAKEUP_MOD_H 6             //
// Number of stored samples        //
#define NUM_STORED_SAMPLES 8       //
/////////////////////////////////////

// Minimum samples before providing estimations
#define MIN_SAMPLES 2

// Screen dimensions - not chalk for now
#if defined(PBL_PLATFORM_EMERY)
  #define DISPLAY_W 200
  #define DISPLAY_H 228
#else
  #define DISPLAY_W 144
  #define DISPLAY_H 168
#endif

// Testing area
// #define WIPE_ON_LAUNCH true // Wipe all data on launch
// #define TEST_DATA true      // Use test data instead of real data
// #define SAVE_TEST_DATA true // Load some sample data into persist
#define DEBUG_LOGS true     // Log calculations