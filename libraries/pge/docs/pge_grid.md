# PGE Grid Documentation

`pge_grid.h` provides convenience functions for games involving grid maths.

1. Include the header file:

        #include "pge/modules/pge_grid.h"

2. Set the size of a tile, in pixels:

        pge_grid_set_tile_size(12);

3. Use the tile size set to get the best fit grid dimensions for the Pebble
   screen. This is the largest size that will fix but not spill over, so some
   margins may be visible if the size does not fit the screen exactly.

        GSize grid_size = pge_grid_get_grid_dimensions();

4. Optionally draw all the grid lines (white or black) in your game's
   PGERenderHandler:

        void render_handler(GContext *ctx) {
          pge_grid_draw_lines(ctx, GColorWhite);
        }

5. Move a `GPoint` by a certain number of grid spaces:

        // Declare a point, preferably grid-aligned
        GPoint now_point = GPoint(0, 0);

        // Move by (1, 2)
        GPoint new_point = GPoint pge_grid_move(now, 1, 2);