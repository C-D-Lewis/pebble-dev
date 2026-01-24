#pragma once

#include <pebble.h>

#include "modules/scalable.h"

///////////////////////////////////////////
// DO NOT CHANGE WITHOUT DUE CARE        //
//                                       //
// Hours between wakeups (check strings) //
#define WAKEUP_MOD_H 6                   //
// Number of stored samples              //
#define NUM_SAMPLES 16                   //
///////////////////////////////////////////

// MUST match package.json
#define VERSION "1.20.0"
// Minimum samples before providing estimations
#define MIN_SAMPLES 2
// Min samples before graph is shown
#define MIN_SAMPLES_FOR_GRAPH 4
// Maximum number of extra minutes if a collision
#define EXTRA_MINUTES_MAX 10
// Multipler for elevated rate alarm
#define EVEVATED_RATE_MULT 3
// Minimum negative charge_diff to count as charging
#define MIN_CHARGE_AMOUNT 10

// MenuLayer single row height
#define ROW_HEIGHT_SMALL scl_y(220)
// MenuLayer two row height
#define ROW_HEIGHT_LARGE scl_y(300)
// Action bar width
#define ACTION_BAR_W scl_x(70)
// Action bar hint width
#define HINT_W scl_x(70)
// Action bar hint height
#define HINT_H scl_y(200)

#if defined(PBL_PLATFORM_EMERY)
// Line width
#define LINE_W 2
#else
#define LINE_W 1
#endif

///////////// Testing area ////////////////
//                                       //
// #define USE_TEST_DATA true            //
//                                       //
// #define SAVE_TEST_DATA true           //
//                                       //
// #define LOG_STATE true                //
//                                       //
// #define WAKEUP_NEXT_MINUTE true       //
//                                       //
// #define TEST_COLLISION true           //
//                                       //
// #define TEST_TIMELINE_PIN true        //
//                                       //
// #define TEST_FORCE_SCALING true       //
//                                       //
// #define TEST_OVERESTIMATION true      //
//                                       //
///////////////////////////////////////////
