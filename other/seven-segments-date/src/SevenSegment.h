/*
 * SevenSegment scalable UI element for Pebble
 * Chris Lewis
 * Free for any use with attribution
 *
 * Segment layout (specified for animation with 1 (on) or 0 (off) in int[7]{})
 *
 *     0
 *    ___
 *   |   |
 * 5 | 6 | 1
 *   |___| 
 *   |   |
 * 4 |   | 2
 *   |___| 
 * 
 *     3
 */

#pragma once

#include <pebble.h>

typedef struct {
  InverterLayer *layer_array[7];
  GPoint origin;
  GSize seg_dims; //Length-Thickness
  int value;
  int anim_duration;
  int seg_delay;
  int last_state[7];
} SevenSegment;

/**
 * Create a SevenSegment digit, and add it to a parent Window
 */
SevenSegment* seven_segment_create(GPoint origin, GSize segment_dimensions, int anim_duration, int seg_delay, Window *parent);

/**
 * Destroy a SevenSegment digit
 */
void seven_segment_destroy(SevenSegment *this);

/**
 * Set the value of the digit, from 0 - C (12)
 * Invlid values hide all segments
 */
void seven_segment_set_value(SevenSegment *this, int value);

/**
 * Get the current value of the SevenSegment digit
 */
int seven_segment_get_value(SevenSegment *this);
