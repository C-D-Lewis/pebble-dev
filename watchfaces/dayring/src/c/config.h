#pragma once

#include <pebble.h>

// Platform-dependant values
#if defined(PBL_PLATFORM_EMERY)
  #define DISPLAY_W 200
  #define DISPLAY_H 228
#elif defined(PBL_PLATFORM_CHALK)
  #define DISPLAY_W 180
  #define DISPLAY_H 180
#else
  #define DISPLAY_W 144
  #define DISPLAY_H 168
#endif
