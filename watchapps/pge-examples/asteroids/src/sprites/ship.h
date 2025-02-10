#pragma once

#include "../main.h"

#define SHIP_MOVE_SPEED 3

typedef struct {
  PGESprite *sprite;
} Ship;

Ship* ship_create(GPoint pos);

void ship_destroy(Ship* this);

void ship_draw(Ship *this, GContext *ctx);

void ship_move(Ship *this, int dx);

GPoint ship_get_position(Ship *this);

GRect ship_get_bounds(Ship *this);