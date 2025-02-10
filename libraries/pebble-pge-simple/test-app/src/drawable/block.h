#pragma once

#include <pebble.h>
#include <pebble-isometric/pebble-isometric.h>

#define BLOCK_HEIGHT 20
#define BLOCK_SIZE GSize(BLOCK_HEIGHT, BLOCK_HEIGHT)

typedef struct {
  Vec3 origin;
  int dz;
  GColor side_color;
  GColor face_color;
} Block;

Block* block_create(Vec3 origin);

void block_destroy(Block *this);

void block_logic(Block *this);

void block_render(Block *this);

void block_set_colors(Block *this, GColor side_color, GColor face_color);

void block_set_z(Block *this, int z);
