#pragma once

#include <pebble.h>

#define VERBOSE true
#define VERSION "3.8"

typedef enum {
  LineTypeBakerloo = 0,
  LineTypeCentral,
  LineTypeCircle,
  LineTypeDistrict,
  LineTypeDLR,
  LineTypeElizabeth,
  LineTypeHammersmithAndCity,
  LineTypeJubilee,
  LineTypeLiberty,
  LineTypeLioness,
  LineTypeMetropolitan,
  LineTypeMildmay,
  LineTypeNorthern,
  LineTypePicadilly,
  LineTypeSuffragette,
  LineTypeVictoria,
  LineTypeWaterlooAndCity,
  LineTypeWeaver,
  LineTypeWindrush,

  LineTypeMax
} LineType;
