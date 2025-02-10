#include "cloud.h"

Cloud *cloud_create(Vec3 origin, GSize size, Vec3 sky_box) {
  Cloud *this = (Cloud*)malloc(sizeof(Cloud));
  this->block = block_create(origin, size, COLOR_CLOUD);
  this->sky_box = sky_box;
  this->visible = false;

  return this;
}

void cloud_destroy(Cloud *this) {
  block_destroy(this->block);
  free(this);
}

void cloud_logic(Cloud *this) {
  // Drift across the sky...
  if(this->visible) {
    if(this->block->position.y > this->sky_box.y) {
      // Poof
      this->visible = false;
    } else {
      // Move once a second
      this->block->position.y++;
    }
  }
}

void cloud_render(Cloud *this) {
  if(this->visible) {
    block_render(this->block);
  }
}

void cloud_spawn(Cloud *this) {
  this->visible = true;
  this->block->position.y = 0;
  this->block->position.x = rand() % this->sky_box.y;
}

bool cloud_get_visible(Cloud *this) {
  return this->visible;
}

int cloud_get_struct_size() {
  return sizeof(Cloud) + sizeof(Block);
}