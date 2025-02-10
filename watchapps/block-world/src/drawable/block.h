#pragma once

#include <pebble.h>

#include "../main.h"
#include "../pge/modules/pge_isometric.h"

typedef struct {
  Vec3 position;
  GSizeSmall size;
  GColor color;
} __attribute__((__packed__)) Block;

Block* block_create(Vec3 vec, GSize size, GColor color);

void block_destroy(Block *this);

void block_render(Block *this);

void block_set_color(Block *this, GColor color);

GColor block_get_color(Block *this);

void block_move(Block *this, Vec3 delta);

Vec3 block_get_position(Block *this);

void block_set_position(Block *this, Vec3 pos);

int block_get_struct_size();