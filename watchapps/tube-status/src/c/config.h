#pragma once

#include <pebble.h>

#define VERBOSE true
#define VERSION "3.4"

typedef enum {
  LineTypeBakerloo = 0,
  LineTypeCentral,
  LineTypeCircle,
  LineTypeDistrict,
  LineTypeHammersmithAndCity,
  LineTypeJubilee,
  LineTypeMetropolitan,
  LineTypeNorthern,
  LineTypePicadilly,
  LineTypeVictoria,
  LineTypeWaterlooAndCity,

  LineTypeMax
} LineType;
