#pragma once
 
#include <pebble.h>

#define ISOMETRIC_OPTIMIZE_FILL_BOX // Only draw front of a box to save time

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
} __attribute__((__packed__)) Vec3;

#define Vec3(x, y, z) ((Vec3){(x),(y),(z)})

/**
 * Begin isometric drawing. Sets up internal frame buffer data.
 * Call this first.
 *
 * Optionally use the returned GBitmap pointer to further manipulate the framebuffer
 */
GBitmap* isometric_begin(GContext *ctx);

/**
 * Stop isometric drawing.
 * Call this last before the end of the implementing LayerUpdateProc
 */
void isometric_finish(GContext *ctx);

/**
 * Toggle whether isometric projection is enabled
 * Turn off for traditional top-down view
 */
void isometric_set_enabled(bool b);

/**
 * Set the screen space drawing offset
 */
void isometric_set_projection_offset(GPoint offset);

/**
 * Project any point from screen space to isometric space
 */
GPoint isometric_project(Vec3 input);

/**
 * Draw an isometric rectangle
 */
void isometric_draw_rect(Vec3 origin, GSize size, GColor color);

/**
 * Fill an isometric rectangle
 */
void isometric_fill_rect(Vec3 origin, GSize size, GColor color);

/**
 * Fill an isometric box with z height
 */
void isometric_fill_box(Vec3 origin, GSize size, int z_height, GColor color);

/**
 * Draw an isometric box with z height
 */
void isometric_draw_box(Vec3 origin, GSize size, int z_height, GColor color);

/**
 * Draw a single projected isometric pixel
 */
void isometric_draw_pixel(Vec3 point, GColor color);

/**
 * Draw an isometric rectangle filled with a texture
 */
void isometric_fill_textured_rect(Vec3 origin, GBitmap *texture);
