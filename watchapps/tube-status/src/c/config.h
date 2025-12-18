#pragma once

#include <pebble.h>

// Aplite should not go over 25 lines, add a condition we if add a transit system that would ever have more than 25
// outages at once. 
// If regular/nightly service closures exceed this amount for a line, they may need filtered out in the backend.
#define MAX_LINES 25

// Status severity levels for visual indication
typedef enum {
  StatusSeverityGood = 0,
  StatusSeverityWarning = 1,
  StatusSeveritySevere = 2
} StatusSeverity;
