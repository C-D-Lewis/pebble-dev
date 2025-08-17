#pragma once

#include <pebble.h>

#define DEBUG                false

#define NUM_BEAMS            4
#define NUM_CHARS            5

#define Y_OFFSET             53

#define SECONDS_Y_OFFSET     105
#define SECONDS_HEIGHT       5

#define DATE_BUFFER_SIZE     32
#define DATE_HEIGHT          30
#define DATE_X_OFFSET        PBL_IF_ROUND_ELSE(0, 45)

#define BEAM_SIZE            GSize(30, 101)
#define BEAM_X_OFFSET        14

#define BT_Y_OFFSET          140

#define BATTERY_HEIGHT       3

#define DIGIT_SIZE           GSize(50, 60)

#define HOURS_TENS_X_OFFSET  PBL_IF_ROUND_ELSE(5, -13)
#define HOURS_UNITS_X_OFFSET PBL_IF_ROUND_ELSE(39, 21)
#define COLON_X_OFFSET       HOURS_UNITS_X_OFFSET + 9
#define MINS_TENS_X_OFFSET   PBL_IF_ROUND_ELSE(81, 63)
#define MINS_UNITS_X_OFFSET  PBL_IF_ROUND_ELSE(115, 97)