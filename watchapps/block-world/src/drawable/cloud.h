#pragma once

#include <pebble.h>

#include "../main.h"
#include "block.h"
#include "../pge/modules/pge_isometric.h"

typedef struct {
  Block *block;
  Vec3 sky_box;
  bool visible;
} __attribute__((__packed__)) Cloud;

Cloud *cloud_create(Vec3 origin, GSize size, Vec3 sky_box);

void cloud_destroy(Cloud *this);

void cloud_logic(Cloud *this);

void cloud_render(Cloud *this);

void cloud_spawn(Cloud *this);

bool cloud_get_visible(Cloud *this);

int cloud_get_struct_size();