#include "block.h"

Block* block_create(Vec3 vec, GSize size, GColor color) {
  Block *this = (Block*)malloc(sizeof(Block));
  this->position = vec;
  this->size.w = size.w;
  this->size.h = size.h;
  this->color = color;

  return this;
}

void block_destroy(Block *this) {
  free(this);
}

void block_render(Block *this) {
  if(!gcolor_equal(this->color, COLOR_INVISIBLE)) {
    pge_isometric_fill_box(this->position, GSize(this->size.w, this->size.h), this->size.w, this->color);
#ifdef OUTLINES
    pge_isometric_draw_box(this->position, GSize(this->size.w, this->size.h), this->size.w, GColorDarkGray);
#endif
  }
}

void block_set_color(Block *this, GColor color) {
  this->color = color;
}

GColor block_get_color(Block *this) {
  return this->color;
}

void block_move(Block *this, Vec3 delta) {
  this->position.x += delta.x;
  this->position.y += delta.y;
  this->position.z += delta.z;
}

Vec3 block_get_position(Block *this) {
  return this->position;
}

void block_set_position(Block *this, Vec3 pos) {
  this->position = pos;
}

int block_get_struct_size() {
  return sizeof(Block);
}