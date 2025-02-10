#include "block.h"

Block* block_create(Vec3 origin) {
  Block *this = (Block*)malloc(sizeof(Block));
  this->origin = origin;
  this->dz = 0;

  return this;
}

void block_destroy(Block *this) {
  free(this);
}

void block_logic(Block *this) {
  // Moved by Animation in main.c
}

void block_render(Block *this) {
#if defined(PBL_BW)
  isometric_fill_box(this->origin, BLOCK_SIZE, BLOCK_HEIGHT, GColorBlack);
  isometric_draw_box(this->origin, BLOCK_SIZE, BLOCK_HEIGHT, GColorWhite);
  isometric_fill_rect(Vec3(this->origin.x, this->origin.y, this->origin.z + BLOCK_HEIGHT), BLOCK_SIZE, GColorWhite);
#elif defined(PBL_COLOR)
  isometric_fill_box(this->origin, BLOCK_SIZE, BLOCK_HEIGHT, this->side_color);
  isometric_fill_rect(Vec3(this->origin.x, this->origin.y, this->origin.z + BLOCK_HEIGHT), BLOCK_SIZE, this->face_color);
#endif
}

void block_set_colors(Block *this, GColor side_color, GColor face_color) {
  this->side_color = side_color;
  this->face_color = face_color;
}

void block_set_z(Block *this, int z) {
  this->origin.z = z;
}
