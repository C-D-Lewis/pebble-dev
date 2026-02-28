#pragma once

#include <pebble.h>

#include "modules/scalable.h"

//////////////////////////////////////////////////
// DO NOT CHANGE WITHOUT DUE CARE               //
//                                              //
// Hours between wakeups (check strings and JS) //
#define WAKEUP_MOD_H 6                          //
// Number of stored samples  (4 days)           //
#define NUM_SAMPLES 16                          //
//////////////////////////////////////////////////

// ! MUST match package.json
// ! Check Aplite works before releasing
// ! Comment out INFO logs
// ! Check TS test flags
#define VERSION "1.22.0"
// Minimum samples before providing estimations
#define MIN_SAMPLES 2
// Min samples before graph is shown
#define MIN_SAMPLES_FOR_GRAPH 4
// Maximum number of extra minutes if a collision
#define WAKEUP_WINDOW_M 10
// Multipler for elevated rate alarm
#define EVEVATED_RATE_MULT 3
// Minimum negative charge_diff to count as charging
#define MIN_CHARGE_AMOUNT 10
/** Maximum synced items - 120 days / 4 months (must match constants.ts) */
#define MAX_SYNC_ITEMS 480

///////////// Testing area ////////////////
//                                       //
// #define USE_TEST_DATA true            //
//                                       //
// #define SAVE_TEST_DATA true           //
//                                       //
// #define SYNC_TEST_DATA true           //
//                                       //
// #define LOG_STATE true                //
//                                       //
// #define WAKEUP_NEXT_MINUTE true       //
//                                       //
// #define TEST_COLLISION true           //
//                                       //
// #define TEST_TIMELINE_PIN true        //
//                                       //
///////////////////////////////////////////

//////////////// Features //////////
#if !defined(PBL_PLATFORM_APLITE) //
#define FEATURE_ANIMATIONS true   //
#define FEATURE_SYNC true         //
#endif                            //
////////////////////////////////////

// Layout values
// MenuLayer single row height
#define ROW_HEIGHT_SMALL scl_y(220)
// MenuLayer two row height
#define ROW_HEIGHT_LARGE scl_y(300)
// Action bar width
#define ACTION_BAR_W scl_x_pp({.o = 60, .c = 90, .e = 70})
// Action bar hint width
#define HINT_W ACTION_BAR_W
// Action bar hint height
#define HINT_H scl_y_pp({.o = 200, .c = 80, .e = 200})
// Header inset
#define HEADER_INSET scl_y(110)

// Per-platform layout values
#if defined(PBL_PLATFORM_EMERY)
// Line width
#define LINE_W 2
#else
#define LINE_W 1
#endif
