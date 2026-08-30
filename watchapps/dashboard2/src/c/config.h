#pragma once

#include <pebble.h>

// RELEASE CHECKLIST
// - Check compatible protocol version is correct
// - Disable TEST flags



////////////////////////////////////////// TESTING AREA ////////////////////////////////////////////
// Use test data
// #define USE_TEST_DATA true
// Expand the tray automatically
// #define TEST_EXPAND_TRAY true

///////////////////////////////////////////// Behavior /////////////////////////////////////////////

// Compatible protocol version, set in Android when protocol changes
#define COMPAT_PROTOCOL_VERSION 3
// AppMessage inbox size
#define COMM_INBOX_SIZE 256
// AppMessage outbox size
#define COMM_OUTBOX_SIZE 128

/////////////////////////////////////////////// Types //////////////////////////////////////////////

typedef enum {
  SyncStateInitial = 0,
  SyncStateOutOfDate,
  SyncStateSuccess
} SyncState;

// Lines up with main window selection index
typedef enum {
  ToggleTypeNothing = -1,
  ToggleTypeFindPhone = 0,
  ToggleTypeCycleRinger = 1,

  TogglesMax
} ToggleType;

// Length of chars for each toggle's type+state
#define TOGGLE_ITEM_LEN 2
// Length of meaningful string in toggles_order (2 * num toggles)
#define TOGGLES_STRLEN TOGGLE_ITEM_LEN * TogglesMax

// NOT persisted data
typedef struct {
  SyncState sync_state;
  int compat_protocol_version;
  char toggle_order[TOGGLES_STRLEN + 1];   // 2c x X toggles + null
  char device_name[32];
  char battery_perc[4];    // '100'
  char free_space[12];     // '999.99 GB'
  char free_space_perc[4]; // '100'
  char next_alarm[16];     // 'Wed 08:00'
  bool is_charging;

} AppState;

// Toggle states, compared with strings so can't be enum
#define TOGGLE_STATE_OFF '0'
#define TOGGLE_STATE_ON '1'
#define TOGGLE_STATE_RINGER_LOUD '2'
#define TOGGLE_STATE_RINGER_VIBRATE '3'
#define TOGGLE_STATE_RINGER_DND '4'
