#pragma once

#include "../main.h"

typedef struct {
  PGESprite *sprite;
} Shot;

Shot* shot_create(GPoint pos);

void shot_destroy(Shot* this);

void shot_draw(Shot *this, GContext *ctx);

void shot_logic(Shot *this);

void shot_move(Shot *this, int dy);

GPoint shot_get_position(Shot *this);

GRect shot_get_bounds(Shot *this);
