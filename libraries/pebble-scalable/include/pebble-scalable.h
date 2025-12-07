#pragma once

#include <pebble.h>

///////////////////////////////////////////// Geometry /////////////////////////////////////////////

/**
 * Get an X coordinate as a percentage of the screen width.
 */
int scalable_x(int perc);

/**
 * Get a Y coordinate as a percentage of the screen height.
 */
int scalable_y(int perc);

/**
 * Get a GRect with all values as percentages of the screen.
 */
GRect scalable_grect(int x_perc, int y_perc, int w_perc, int h_perc);

/**
 * Manually adjust a GRect position by a number of pixels.
 */
GRect scalable_nudge_xy(GRect r, int dx, int dy);

/**
 * Manually adjust a GRect size by a number of pixels.
 */
GRect scalable_nudge_wh(GRect r, int dw, int dh);

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
 * Set fonts to use for 'small' size on various known screensizes.
 *
 * The intention here is to provide an optional 'font theme' on a per-size basis.
 *
 * Q: Can this be variadic for the future? Data structure?
 */
void scalable_set_small_fonts(GFont *regular, GFont *chalk, GFont *emery);

/**
 * Set fonts to use for 'medium' size on various known screensizes.
 */
void scalable_set_medium_fonts(GFont *regular, GFont *chalk, GFont *emery);

/**
 * Set fonts to use for 'large' size on various known screensizes.
 */
void scalable_set_large_fonts(GFont *regular, GFont *chalk, GFont *emery);

/**
 * Get the small font for the current platform, if previously set.
 */
GFont scalable_get_small_font();

/**
 * Get the medium font for the current platform, if previously set.
 */
GFont scalable_get_medium_font();

/**
 * Get the large font for the current platform, if previously set.
 */
GFont scalable_get_large_font();
