#pragma once

#include <pebble.h>

#if defined(APP_VARIANT_TUBE_STATUS)
// Tube Status does not use the transit system selection feature so no slots needed
#define MAX_TRANSIT_SYSTEMS 0
#else
// This should be equal to the known number of transit system backends on the phone side JS
#define MAX_TRANSIT_SYSTEMS 3
#endif

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
