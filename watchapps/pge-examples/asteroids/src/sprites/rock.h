#pragma once

#include "../main.h"

typedef struct {
  PGESprite *sprite;
} Rock;

Rock* rock_create(GPoint pos);

void rock_destroy(Rock* this);

void rock_draw(Rock *this, GContext *ctx);

void rock_logic(Rock *this);

void rock_move(Rock *this, int dy);

GPoint rock_get_position(Rock *this);

GRect rock_get_bounds(Rock *this);
