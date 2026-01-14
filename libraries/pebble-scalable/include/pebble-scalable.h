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
 * Get an X coordinate as a thousandth of the screen width, same on all platforms.
 */
int scl_x(int t_perc);

/**
 * Get a Y coordinate as a thousandth of the screen height, same on all platforms.
 */
int scl_y(int t_perc);

/**
 * As scl_x, but with platform-specific values.
 */
int _scl_x_pp_impl(SV values);
#define scl_x_pp(...) _scl_x_pp_impl((SV)__VA_ARGS__)

/**
 * As scl_y, but with platform-specific values.
 */
int _scl_y_pp_impl(SV values);
#define scl_y_pp(...) _scl_y_pp_impl((SV)__VA_ARGS__)

/**
 * Get a scaled GRect with values as thousandths of screen dimensions, same on all platforms.
 *
 * For per-platform values, construct a GRect using scl_x_pp and scl_y_pp as needed.
 */
GRect scl_grect(int x_t_perc, int y_t_perc, int w_t_perc, int h_t_perc);

/**
 * Center a GRect in the X axis based on its size.
 */
GRect scl_center_x(GRect r);

/**
 * Center a GRect in the Y axis based on its size.
 */
GRect scl_center_y(GRect r);

/**
 * Center a GRect in both axes.
 */
GRect scl_center(GRect r);

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
void _scl_set_fonts_impl(int size_id, SF fonts);
#define scl_set_fonts(id, ...) _scl_set_fonts_impl(id, (SF)__VA_ARGS__)

/**
 * Get a font for the current platform by size ID, if previously set.
 */
GFont scl_get_font(int size_id);

////////////////////////////////////////////// Images //////////////////////////////////////////////

// Images can use the SDK's resource tagging system, and their position can use
// the same scalable functions.
