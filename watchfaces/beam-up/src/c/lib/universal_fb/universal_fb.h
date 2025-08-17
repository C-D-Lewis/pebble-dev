/**
 * Universal set, get, and swap colors for Pebble SDK 3.0
 * Author: Chris Lewis
 * License: MIT
 */

#pragma once

#include <pebble.h>

/*
 * Get the GColor of a given point
 * Returns GColorClear if out of bounds
 */
GColor universal_fb_get_pixel_color(GBitmapDataRowInfo info, GRect bounds, GPoint point);

/**
 * Set a pixel's GColor
 */
void universal_fb_set_pixel_color(GBitmapDataRowInfo info, GRect bounds, GPoint point, GColor color);

/**
 * Swap two colors between each other.
 * c2 will only replace c1. No other inversion will occur
 */
void universal_fb_swap_colors(GBitmap *fb, GRect bounds, GColor c1, GColor c2);
