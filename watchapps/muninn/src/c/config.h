#pragma once

#include <pebble.h>

// DO NOT CHANGE WITHOUT MIGRATION //
// Hours between wakeups           //
#define WAKEUP_MOD_H 6             //
// Number of stored samples        //
#define NUM_SAMPLES 8              //
/////////////////////////////////////

// MUST match package.json
#define VERSION "1.11.0"
// Minimum samples before providing estimations
#define MIN_SAMPLES 2
// Maximum number of extra minutes if a collision
#define EXTRA_MINUTES_MAX 10
// Multipler for elevated rate alarm
#define EVEVATED_RATE_MULT 2

// Platform-dependant values
#if defined(PBL_PLATFORM_EMERY)
  #define DISPLAY_W 200
  #define DISPLAY_H 228
#else
  #define DISPLAY_W 144
  #define DISPLAY_H 168
#endif

///////////// Testing area /////////////
//                                    //
// #define USE_TEST_DATA true         //
//                                    //
// #define SAVE_TEST_DATA true        //
//                                    //
#define LOG_STATE true             //
//                                    //
// #define WAKEUP_NEXT_MINUTE true    //
//                                    //
// #define TEST_COLLISION true        //
//                                    //
// #define TEST_TIMELINE_PIN true     //
//                                    //
// #define TEST_FORCE_SCALING true    //
//                                    //
////////////////////////////////////////
