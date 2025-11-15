#pragma once

#include <pebble.h>

// Hours between wakeups
#define WAKEUP_INTERVAL_S (60 * 60 * 12) // 12 hours
// #define WAKEUP_INTERVAL_S (60 * 2) // 2 mins

// Number of stores samples
#define NUM_STORED_SAMPLES 6 // Careful changing this

// Screen width
#define WIDTH PBL_IF_ROUND_ELSE(180, 144)

// Screen height
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)

// Wipe data on launch (debugging use)
// #define WIPE_ON_LAUNCH true

// Test data
// #define TEST_DATA true
