#include "drawing.h"

void drawing_draw_square(Vec3 pos, GColor color) {
  isometric_fill_box(pos, BOX_SIZE, B_H, color);
  isometric_draw_box(pos, BOX_SIZE, B_H, GColorBlack, false);
}
