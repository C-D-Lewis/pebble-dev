#pragma once

#include <pebble.h>

/**
 * Set the side size of the grid tiles
 */
void pge_grid_set_tile_size(int new_tile_size);

/**
 * Get the grid dimensions of the screen with the current tile size
 */
GSize pge_grid_get_grid_dimensions();

/**
 * Draw the current grid lines
 */
void pge_grid_draw_lines(GContext *ctx, GColor line_color);

/**
 * Move a screen-wise point by a number of grid-wise spaces
 */
GPoint pge_grid_move(GPoint now, int grid_dx, int grid_dy);
