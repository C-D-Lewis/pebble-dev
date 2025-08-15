#pragma once

#include "segment.h"

#include "../config.h"

typedef struct {
  Segment *segments[15];
  GPoint origin;
  int value;
} Digit;

Digit* digit_create(GPoint origin, int initial_value);

void digit_destroy(Digit *this);

void digit_set_value(Digit *this, int new_value);

void digit_logic(Digit *this);

void digit_render(Digit *this);

void digit_set_colors(Digit *this, GColor side_color, GColor face_color);

bool digit_is_animating(Digit *this);
