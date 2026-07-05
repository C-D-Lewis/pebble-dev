#pragma once

#include <pebble.h>

// RELEASE CHECKLIST
// Android min. version values changed?

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
