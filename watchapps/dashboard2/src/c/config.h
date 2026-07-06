#pragma once

#include <pebble.h>

// RELEASE CHECKLIST
// - Update defined versions
// - Android min. version values changed?

// Current watchapp version, always update
#define COMPAT_PROTOCOL_VERSION 1

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
  ToggleFindPhone = 0,
  ToggleCycleRinger,

  TogglesMax
} Toggle;
