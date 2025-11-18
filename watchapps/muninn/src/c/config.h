#pragma once

#include <pebble.h>

// Hours between wakeups
#define WAKEUP_H 12

// Number of stores samples
#define NUM_STORED_SAMPLES 6 // Careful changing this

// Screen width
#define WIDTH PBL_IF_ROUND_ELSE(180, 144)

// Screen height
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)

// Testing area
// #define WIPE_ON_LAUNCH true // Wipe all data on launch
// #define TEST_DATA true      // Use test data instead of real data
// #define LOAD_DATA true      // Load some sample data into persist
