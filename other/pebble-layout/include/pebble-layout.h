#pragma once

#include <pebble.h>

/**
 * Get an x coordinate as a percentage of the screen width.
 */
int pl_x(int perc);

/**
 * Get a y coordinate as a percentage of the screen height.
 */
int pl_y(int perc);

/**
 * Get a GRect with all values as percentages of the screen.
 */
GRect pl_grect(int x_perc, int y_perc, int w_perc, int h_perc);

/**
 * Manually adjust a GRect position by a number of pixels.
 */
GRect pl_nudge_xy(GRect r, int dx, int dy);

/**
 * Manually adjust a GRect size by a number of pixels.
 */
GRect pl_nudge_wh(GRect r, int dw, int dh);

/**
 * Set fonts to use for 'small' size on various known screensizes.
 *
 * The intention here is to provide an optional 'font theme' on a per-size basis.
 *
 * Q: Can this be variadic for the future? Data structure?
 */
void pl_set_small_fonts(GFont *regular, GFont *chalk, GFont *emery);

/**
 * Set fonts to use for 'medium' size on various known screensizes.
 */
void pl_set_medium_fonts(GFont *regular, GFont *chalk, GFont *emery);

/**
 * Set fonts to use for 'large' size on various known screensizes.
 */
void pl_set_large_fonts(GFont *regular, GFont *chalk, GFont *emery);

/**
 * Get the small font for the current platform, if previously set.
 */
GFont pl_get_small_font();

/**
 * Get the medium font for the current platform, if previously set.
 */
GFont pl_get_medium_font();

/**
 * Get the large font for the current platform, if previously set.
 */
GFont pl_get_large_font();
