#include "pge_grid.h"

static int s_tile_size = 17;  //Default value for whole tiles at least horizontally

void pge_grid_set_tile_size(int new_tile_size) {
  s_tile_size = new_tile_size;
}

GSize pge_grid_get_grid_dimensions() {
  GSize result = GSize(144 / s_tile_size, 168 / s_tile_size);
  if(result.w * s_tile_size > 144) {
    result.w--;
  }
  if(result.h * s_tile_size > 168) {
    result.h--;
  }
  return result;
}

void pge_grid_draw_lines(GContext *ctx, GColor line_color) {
  GSize dims = pge_grid_get_grid_dimensions();
  graphics_context_set_stroke_color(ctx, line_color);

  // Draw horizontal lines
  for(int grid_y = 0; grid_y < dims.h; grid_y++) {
    graphics_draw_line(ctx, GPoint(0, grid_y * s_tile_size), GPoint(dims.w * s_tile_size, grid_y * s_tile_size));
  }
  graphics_draw_line(ctx, GPoint(0, dims.h * s_tile_size), GPoint(dims.w * s_tile_size, dims.h * s_tile_size));

  // Draw vertical lines
  for(int grid_x = 0; grid_x < dims.w; grid_x++) {
    graphics_draw_line(ctx, GPoint(grid_x * s_tile_size, 0), GPoint(grid_x * s_tile_size, dims.h * s_tile_size));
  }
  graphics_draw_line(ctx, GPoint(dims.w * s_tile_size, 0), GPoint(dims.w * s_tile_size, dims.h * s_tile_size));
}

GPoint pge_grid_move(GPoint now, int grid_dx, int grid_dy) {
  now.x += grid_dx * s_tile_size;
  now.y += grid_dy * s_tile_size;

  return now;
}