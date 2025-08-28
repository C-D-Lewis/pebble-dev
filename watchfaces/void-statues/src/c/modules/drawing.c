#include "drawing.h"

static GColor s_box_color, s_shadow_color, s_clarity_color;
static bool s_is_connected = false;

static void draw_box(Vec3 pos) {
  if (s_is_connected) {
    isometric_fill_box(Vec3(pos.x, pos.y, pos.z), BOX_SIZE, B_H, s_box_color);
  } else {
    isometric_draw_box(Vec3(pos.x, pos.y, pos.z), BOX_SIZE, B_H, s_box_color, false);
  }
}

static void draw_shadow(Vec3 pos) {
  isometric_fill_rect(Vec3(pos.x, pos.y, pos.z), BOX_SIZE, s_shadow_color);
}

static Vec3 get_block_position(int cell_index, GPoint pos) {
  switch(cell_index) {
    case 0: return Vec3(pos.x, pos.y + (2 * B_W), 4 * B_H);
    case 1: return Vec3(pos.x, pos.y + B_W, 4 * B_H);
    case 2: return Vec3(pos.x, pos.y, 4 * B_H);
    case 3: return Vec3(pos.x, pos.y + (2 * B_W), 3 * B_H);
    case 4: return Vec3(pos.x, pos.y + B_W, 3 * B_H);
    case 5: return Vec3(pos.x, pos.y, 3 * B_H);
    case 6: return Vec3(pos.x, pos.y + (2 * B_W), 2 * B_H);
    case 7: return Vec3(pos.x, pos.y + B_W, 2 * B_H);
    case 8: return Vec3(pos.x, pos.y, 2 * B_H);
    case 9: return Vec3(pos.x, pos.y + (2 * B_W), B_H);
    case 10: return Vec3(pos.x, pos.y + B_W, B_H);
    case 11: return Vec3(pos.x, pos.y, B_H);
    case 12: return Vec3(pos.x, pos.y + (2 * B_W), 0);
    case 13: return Vec3(pos.x, pos.y + B_W, 0);
    case 14: return Vec3(pos.x, pos.y, 0);
    default: {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid cell index: %d", cell_index);
      return Vec3(0, 0, 0);
    }
  }
}

static void draw_digit_blocks(GPoint pos, int cells[15], int number) {
  // Must be drawn bottom right to top left
  if (cells[14] == 1) draw_box(get_block_position(14, pos));
  if (cells[13] == 1) draw_box(get_block_position(13, pos));
  if (cells[12] == 1) draw_box(get_block_position(12, pos));
  if (cells[11] == 1) draw_box(get_block_position(11, pos));
  if (cells[10] == 1) draw_box(get_block_position(10, pos));
  if (number == 0 || number == 6 || number == 8) {
    isometric_fill_box_faces(get_block_position(10, pos), BOX_SIZE, B_H, s_clarity_color, false, true, false);
    isometric_draw_box(get_block_position(10, pos), BOX_SIZE, B_H, s_box_color, true);
  }
  if (cells[9] == 1) draw_box(get_block_position(9, pos));
  if (cells[8] == 1) draw_box(get_block_position(8, pos));
  if (cells[7] == 1) draw_box(get_block_position(7, pos));
  if (number == 0) {
    isometric_fill_box_faces(get_block_position(7, pos), BOX_SIZE, B_H, s_clarity_color, false, true, false);
    isometric_draw_box(get_block_position(7, pos), BOX_SIZE, B_H, s_box_color, true);
  }
  if (cells[6] == 1) draw_box(get_block_position(6, pos));
  if (cells[5] == 1) draw_box(get_block_position(5, pos));
  if (cells[4] == 1) draw_box(get_block_position(4, pos));
  if (number == 0 || number == 8 || number == 9) {
    isometric_fill_box_faces(get_block_position(4, pos), BOX_SIZE, B_H, s_clarity_color, false, true, false);
    isometric_draw_box(get_block_position(4, pos), BOX_SIZE, B_H, s_box_color, true);
  }
  if (cells[3] == 1) draw_box(get_block_position(3, pos));
  if (cells[2] == 1) draw_box(get_block_position(2, pos));
  if (cells[1] == 1) draw_box(get_block_position(1, pos));
  if (cells[0] == 1) draw_box(get_block_position(0, pos));
}

static void draw_digit_shadow(GPoint pos, int cells[15]) {
  if (cells[0] == 1) draw_shadow(Vec3(pos.x + (5 * B_W), pos.y + (2 * B_W), 0));
  if (cells[1] == 1) draw_shadow(Vec3(pos.x + (5 * B_W), pos.y + B_W, 0));
  if (cells[2] == 1) draw_shadow(Vec3(pos.x + (5 * B_W), pos.y, 0));
  if (cells[3] == 1) draw_shadow(Vec3(pos.x + (4 * B_W), pos.y + (2 * B_W), 0));
  if (cells[4] == 1) draw_shadow(Vec3(pos.x + (4 * B_W), pos.y + B_W, 0));
  if (cells[5] == 1) draw_shadow(Vec3(pos.x + (4 * B_W), pos.y, 0));
  if (cells[6] == 1) draw_shadow(Vec3(pos.x + (3 * B_W), pos.y + (2 * B_W), 0));
  if (cells[7] == 1) draw_shadow(Vec3(pos.x + (3 * B_W), pos.y + B_W, 0));
  if (cells[8] == 1) draw_shadow(Vec3(pos.x + (3 * B_W), pos.y, 0));
  if (cells[9] == 1) draw_shadow(Vec3(pos.x + (2 * B_W), pos.y + (2 * B_W), 0));
  if (cells[10] == 1) draw_shadow(Vec3(pos.x + (2 * B_W), pos.y + B_W, 0));
  if (cells[11] == 1) draw_shadow(Vec3(pos.x + (2 * B_W), pos.y, 0));
  if (cells[12] == 1) draw_shadow(Vec3(pos.x + B_W, pos.y + (2 * B_W), 0));
  if (cells[13] == 1) draw_shadow(Vec3(pos.x + B_W, pos.y + B_W, 0));
  if (cells[14] == 1) draw_shadow(Vec3(pos.x + B_W, pos.y, 0));
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
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
      draw_digit_blocks(pos, cells, number);
      break;
    }
    default:
      break;
  }
}

void drawing_set_colors(GColor box_color, GColor shadow_color, GColor clarity_color) {
  s_box_color = box_color;
  s_shadow_color = shadow_color;
  s_clarity_color = clarity_color;
}

void drawing_set_is_connected(bool is_connected) {
  s_is_connected = is_connected;
}
