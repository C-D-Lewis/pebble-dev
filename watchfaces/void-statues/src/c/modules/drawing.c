#include "drawing.h"

static GColor s_box_color, s_shadow_color;

static void draw_box(Vec3 pos) {
  isometric_fill_box(Vec3(pos.x, pos.y, pos.z), BOX_SIZE, H, s_box_color);
  // isometric_draw_box(Vec3(pos.x, pos.y, pos.z), BOX_SIZE, H, GColorWhite);
}

static void draw_shadow(Vec3 pos) {
  isometric_fill_rect(Vec3(pos.x, pos.y, pos.z), BOX_SIZE, s_shadow_color);
}

static void draw_digit_blocks(GPoint pos, int cells[15]) {
  // Must be drawn bottom right to top left
  if (cells[14] == 1) draw_box(Vec3(pos.x, pos.y, 0));
  if (cells[13] == 1) draw_box(Vec3(pos.x, pos.y + W, 0));
  if (cells[12] == 1) draw_box(Vec3(pos.x, pos.y + (2 * W), 0));
  if (cells[11] == 1) draw_box(Vec3(pos.x, pos.y, H));
  if (cells[10] == 1) draw_box(Vec3(pos.x, pos.y + W, H));
  if (cells[9] == 1) draw_box(Vec3(pos.x, pos.y + (2 * W), H));
  if (cells[8] == 1) draw_box(Vec3(pos.x, pos.y, 2 * H));
  if (cells[7] == 1) draw_box(Vec3(pos.x, pos.y + W, 2 * H));
  if (cells[6] == 1) draw_box(Vec3(pos.x, pos.y + (2 * W), 2 * H));
  if (cells[5] == 1) draw_box(Vec3(pos.x, pos.y, 3 * H));
  if (cells[4] == 1) draw_box(Vec3(pos.x, pos.y + W, 3 * H));
  if (cells[3] == 1) draw_box(Vec3(pos.x, pos.y + (2 * W), 3 * H));
  if (cells[2] == 1) draw_box(Vec3(pos.x, pos.y, 4 * H));
  if (cells[1] == 1) draw_box(Vec3(pos.x, pos.y + W, 4 * H));
  if (cells[0] == 1) draw_box(Vec3(pos.x, pos.y + (2 * W), 4 * H));
}

static void draw_digit_shadow(GPoint pos, int cells[15]) {
  if (cells[0] == 1) draw_shadow(Vec3(pos.x + (5 * W), pos.y + (2 * W), 0));
  if (cells[1] == 1) draw_shadow(Vec3(pos.x + (5 * W), pos.y + W, 0));
  if (cells[2] == 1) draw_shadow(Vec3(pos.x + (5 * W), pos.y, 0));
  if (cells[3] == 1) draw_shadow(Vec3(pos.x + (4 * W), pos.y + (2 * W), 0));
  if (cells[4] == 1) draw_shadow(Vec3(pos.x + (4 * W), pos.y + W, 0));
  if (cells[5] == 1) draw_shadow(Vec3(pos.x + (4 * W), pos.y, 0));
  if (cells[6] == 1) draw_shadow(Vec3(pos.x + (3 * W), pos.y + (2 * W), 0));
  if (cells[7] == 1) draw_shadow(Vec3(pos.x + (3 * W), pos.y + W, 0));
  if (cells[8] == 1) draw_shadow(Vec3(pos.x + (3 * W), pos.y, 0));
  if (cells[9] == 1) draw_shadow(Vec3(pos.x + (2 * W), pos.y + (2 * W), 0));
  if (cells[10] == 1) draw_shadow(Vec3(pos.x + (2 * W), pos.y + W, 0));
  if (cells[11] == 1) draw_shadow(Vec3(pos.x + (2 * W), pos.y, 0));
  if (cells[12] == 1) draw_shadow(Vec3(pos.x + W, pos.y + (2 * W), 0));
  if (cells[13] == 1) draw_shadow(Vec3(pos.x + W, pos.y + W, 0));
  if (cells[14] == 1) draw_shadow(Vec3(pos.x + W, pos.y, 0));
}

void drawing_draw_number(int number, GPoint pos) {
  switch (number) {
    case 0: {
      int cells[15] = {
        1, 1, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 1: {
      int cells[15] = {
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 2: {
      int cells[15] = {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        1, 0, 0,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 3: {
      int cells[15] = {
        1, 1, 1,
        0, 0, 1,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 4: {
      int cells[15] = {
        1, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        0, 0, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 5: {
      int cells[15] = {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 6: {
      int cells[15] = {
        1, 1, 1,
        1, 0, 0,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 7: {
      int cells[15] = {
        1, 1, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 8: {
      int cells[15] = {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        1, 0, 1,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    case 9: {
      int cells[15] = {
        1, 1, 1,
        1, 0, 1,
        1, 1, 1,
        0, 0, 1,
        1, 1, 1
      };
      draw_digit_shadow(pos, cells);
      draw_digit_blocks(pos, cells);
      break;
    }
    default:
      break;
  }
}

void drawing_set_colors(GColor box_color, GColor shadow_color) {
  s_box_color = box_color;
  s_shadow_color = shadow_color;
}
