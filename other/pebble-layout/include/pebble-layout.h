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
