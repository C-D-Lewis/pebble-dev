#include "rock.h"

Rock* rock_create(GPoint pos) {
  Rock *this = malloc(sizeof(Rock));
  this->sprite = pge_sprite_create(pos, RESOURCE_ID_ROCK);
  return this;
}

void rock_destroy(Rock* this) {
  pge_sprite_destroy(this->sprite);
  free(this);
}

void rock_draw(Rock *this, GContext *ctx) {
  pge_sprite_draw(this->sprite, ctx);
}

void rock_logic(Rock *this) {
  // Move up
  rock_move(this, 1);
}

void rock_move(Rock *this, int dy) {
  GPoint pos = pge_sprite_get_position(this->sprite);
  pos.y += dy;
  pge_sprite_set_position(this->sprite, pos);
}

GPoint rock_get_position(Rock *this) {
  return pge_sprite_get_position(this->sprite);
}

GRect rock_get_bounds(Rock *this) {
  return pge_sprite_get_bounds(this->sprite);
}