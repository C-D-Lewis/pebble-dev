#pragma once

#include <pebble.h>

#include <pebble-isometric/pebble-isometric.h>

#include "../config.h"

#define SEGMENT_FLY_SPEED 10
#define SEGMENT_Z_MAX 220
#define SEGMENT_DELAY_COEFF 10

typedef struct {
  Vec3 origin;
  bool visible;
  int dz;
  GColor side_color;
  GColor face_color;
  int move_delay;
} Segment;

Segment* segment_create(Vec3 origin);

void segment_destroy(Segment *this);

void segment_logic(Segment *this);

void segment_render(Segment *this);

void segment_set_visible(Segment *this, bool visible, int index);

void segment_set_colors(Segment *this, GColor side_color, GColor face_color);

bool segment_is_at_rest(Segment *this);
