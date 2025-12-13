#pragma once

#include <pebble.h>

///////////////////////////////////////////// Geometry /////////////////////////////////////////////

/**
 * Get an X coordinate as a thousandth of the screen width.
 */
int scalable_x(int t_perc);

/**
 * Get a Y coordinate as a thousandth of the screen height.
 */
int scalable_y(int t_perc);

/**
 * Get a GRect with all values as thousandths of the screen.
 */
GRect scalable_grect(int x_t_perc, int y_t_perc, int w_t_perc, int h_t_perc);

/**
 * Manually adjust a GRect position by a number of pixels.
 */
GRect scalable_nudge(GRect r, int dx, int dy);

/**
 * Manually adjust a GRect position by a number of pixels on regular platforms (original size)
 */
GRect scalable_nudge_regular(GRect r, int dx, int dy);

/**
 * Adjust a GRect position on Emery only.
 */
GRect scalable_nudge_emery(GRect r, int dx, int dy);

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

/**
 * Set fonts to use on various known screensizes accoding to dev-specified IDs, such as small,
 * medium, large, etc.
 *
 * The intention here is to provide an optional 'font theme' on a per-size basis.
 *
 * Keys can be 0 - 15 in value.
 *
 * Q: Can this be variadic for the future? Data structure?
 */
void scalable_set_fonts(int id, GFont *regular, GFont *chalk, GFont *emery);

/**
 * Get a font for the current platform, if previously set.
 */
GFont scalable_get_font(int id);

////////////////////////////////////////////// Images //////////////////////////////////////////////

// Q: Images could vary by color/BW - scope for now is just physical dimensions
