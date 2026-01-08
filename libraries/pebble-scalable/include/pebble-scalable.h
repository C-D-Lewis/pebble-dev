#pragma once

#include <pebble.h>

///////////////////////////////////////////// Constants ////////////////////////////////////////////

#if defined(PBL_PLATFORM_EMERY)
  #define PS_DISP_W 200
  #define PS_DISP_H 228
#elif defined(PBL_PLATFORM_CHALK)
  #define PS_DISP_W 180
  #define PS_DISP_H 180
#else // original sizes (aplite, basalt, diorite, flint)
  #define PS_DISP_W 144
  #define PS_DISP_H 168
#endif

///////////////////////////////////////////// Geometry /////////////////////////////////////////////

// Struct allowing specifying per-platform positions (expandable!)
typedef struct {
  // Original sizes (aplite, basalt, diorite, flint)
  // Note: When the 15th Pebble platform is released this could be a problem
  int o;
  // Chalk
  int c;
  // Emery
  int e;
} SV; // "scalable values"

/**
 * Get an X coordinate as a thousandth of the screen width.
 */
int scalable_x(SV values);

/**
 * Get a Y coordinate as a thousandth of the screen height.
 */
int scalable_y(SV values);

/**
 * Center a GRect in the X axis based on its size.
 */
GRect scalable_center_x(GRect r);

/**
 * Center a GRect in the Y axis based on its size.
 */
GRect scalable_center_y(GRect r);

/**
 * Center a GRect in both axes.
 */
GRect scalable_center(GRect r);

////////////////////////////////////////////// Fonts ///////////////////////////////////////////////

// Struct allowing specifying per-platform fonts (expandable!)
typedef struct {
  // Original sizes (aplite, basalt, diorite, flint)
  GFont o;
  // Chalk
  GFont c;
  // Emery
  GFont e;
} SF; // "scalable fonts"

/**
 * Set fonts to use on various known screen sizes accoding to dev-specified IDs, such as small,
 * medium, large, etc.
 *
 * The intention here is to provide an optional 'font theme' on a per-size basis.
 *
 * Keys can be 0 - 15 in value.
 */
void scalable_set_fonts(int size_id, SF fonts);

/**
 * Get a font for the current platform by size ID, if previously set.
 */
GFont scalable_get_font(int size_id);

////////////////////////////////////////////// Images //////////////////////////////////////////////

// Images can use the SDK's resource tagging system, and their position can use
// the same scalable functions.
