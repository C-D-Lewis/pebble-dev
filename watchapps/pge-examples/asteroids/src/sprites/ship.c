#include "ship.h"

Ship* ship_create(GPoint pos) {
  Ship *this = malloc(sizeof(Ship));
  this->sprite = pge_sprite_create(pos, RESOURCE_ID_SHIP);
  return this;
}

void ship_destroy(Ship* this) {
  pge_sprite_destroy(this->sprite);
  free(this);
}

void ship_draw(Ship *this, GContext *ctx) {
  pge_sprite_draw(this->sprite, ctx);
}

void ship_move(Ship *this, int dx) {
  GPoint pos = pge_sprite_get_position(this->sprite);
  pos.x += dx;
  pge_sprite_set_position(this->sprite, pos);
}

GPoint ship_get_position(Ship *this) {
  return pge_sprite_get_position(this->sprite);
}

GRect ship_get_bounds(Ship *this) {
  return pge_sprite_get_bounds(this->sprite);
}