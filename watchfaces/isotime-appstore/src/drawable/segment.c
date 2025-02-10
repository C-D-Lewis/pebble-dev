#include "segment.h"

Segment* segment_create(Vec3 origin) {
  Segment *this = (Segment*)malloc(sizeof(Segment));
  this->origin = origin;
  this->visible = true;
  this->dz = 0;
  this->move_delay = 0;

  return this;
}

void segment_destroy(Segment *this) {
  free(this);
}

void segment_logic(Segment *this) {
  if(this->move_delay > 0) {
    this->move_delay -= 5;
  } else {
    if((this->visible && this->origin.z > BASE_Y)
    || (!this->visible && this->origin.z > -SEGMENT_Z_MAX)) {
      this->origin.z -= this->dz;
    }
  }
}

void segment_render(Segment *this) {
  GSize size = GSize(BLOCK_SIZE, BLOCK_SIZE);
#if defined(PBL_BW)
  isometric_fill_box(this->origin, size, BLOCK_SIZE, GColorDarkGray);
  isometric_fill_rect(Vec3(this->origin.x, this->origin.y, this->origin.z + BLOCK_SIZE), size, GColorWhite);
#elif defined(PBL_COLOR)
  isometric_fill_box(this->origin, size, BLOCK_SIZE, this->side_color);
  isometric_fill_rect(Vec3(this->origin.x, this->origin.y, this->origin.z + BLOCK_SIZE), size, this->face_color);
#endif
}

void segment_set_visible(Segment *this, bool now_visible, int index) {
  if(this->visible && !now_visible) {
    // Now hiding
    this->dz = SEGMENT_FLY_SPEED;
    this->move_delay = index * SEGMENT_DELAY_COEFF;
  } else if(!this->visible && now_visible) {
    // Show
    this->origin.z = SEGMENT_Z_MAX;
    this->dz = SEGMENT_FLY_SPEED;
    this->move_delay = index * SEGMENT_DELAY_COEFF;
  } else {
    // No movement, conditions match
  }

  this->visible = now_visible;
}

void segment_set_colors(Segment *this, GColor side_color, GColor face_color) {
  this->side_color = side_color;
  this->face_color = face_color;
}

bool segment_is_at_rest(Segment *this) {
  return (this->visible && this->origin.z == BASE_Y) // Supposed to be visible, and is
      || (!this->visible && abs(this->origin.z) >= SEGMENT_Z_MAX);  // Supposed to be invisible and is high up or down low
}
