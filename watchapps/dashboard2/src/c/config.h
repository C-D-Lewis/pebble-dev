#pragma once

#include <pebble.h>

// RELEASE CHECKLIST
// - Check compatible protocol version is correct
// - Disable TEST flags

// Compatible protocol version, set in Android when protocol changes
#define COMPAT_PROTOCOL_VERSION 3

///////////// TESTING AREA /////////////
// Use test data                      //
// #define USE_TEST_DATA true         //
// Expand the tray automatically      //
// #define TEST_EXPAND_TRAY true      //
////////////////////////////////////////

// AppMessage inbox size
#define COMM_INBOX_SIZE 256
// AppMessage outbox size
#define COMM_OUTBOX_SIZE 128

// Lines up with main window selection index
typedef enum {
  ToggleTypeNothing = -1,
  ToggleTypeFindPhone = 0,
  ToggleTypeCycleRinger = 1,

  TogglesMax
} ToggleType;

// Toggle states, compared with strings so can't be enum
#define TOGGLE_STATE_OFF '0'
#define TOGGLE_STATE_ON '1'
#define TOGGLE_STATE_RINGER_LOUD '2'
#define TOGGLE_STATE_RINGER_VIBRATE '3'
#define TOGGLE_STATE_RINGER_DND '4'

typedef enum {
  SyncStateInitial = 0,
  SyncStateOutOfDate,
  SyncStateSuccess
} SyncState;

// Length of chars for each toggle's type+state
#define TOGGLE_FMT_LEN 2
// Length of meaningful string in toggles_order (2 * num toggles)
#define TOGGLES_STRLEN TOGGLE_FMT_LEN * TogglesMax