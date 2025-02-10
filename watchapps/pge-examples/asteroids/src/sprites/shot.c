#include "shot.h"

Shot* shot_create(GPoint pos) {
  Shot *this = malloc(sizeof(Shot));
  this->sprite = pge_sprite_create(pos, RESOURCE_ID_SHOT);
  return this;
}

void shot_destroy(Shot* this) {
  pge_sprite_destroy(this->sprite);
  free(this);
}

void shot_draw(Shot *this, GContext *ctx) {
  pge_sprite_draw(this->sprite, ctx);
}

void shot_logic(Shot *this) {
  // Move up
  shot_move(this, -5);
}

void shot_move(Shot *this, int dy) {
  GPoint pos = pge_sprite_get_position(this->sprite);
  pos.y += dy;
  pge_sprite_set_position(this->sprite, pos);
}

GPoint shot_get_position(Shot *this) {
  return pge_sprite_get_position(this->sprite);
}

GRect shot_get_bounds(Shot *this) {
  return pge_sprite_get_bounds(this->sprite);
}