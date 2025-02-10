#include "pge_isometric.h"

static bool s_enabled = true;
static GPoint s_projection_offset;

static GBitmap *s_fb = NULL;
static GSize s_fb_size;
static GBitmapDataRowInfo s_info;
static uint8_t *s_fb_data = NULL;

static void set_pixel(GPoint pixel, GColor color) {
  universal_fb_set_pixel_color(s_info, gbitmap_get_bounds(s_fb), pixel, color);
}

/**
 * http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
 */
static void bresenham_line(GPoint start, GPoint finish, GColor color) {
  int dx = abs(finish.x - start.x);
  int sx = (start.x < finish.x) ? 1 : -1;
  int dy = abs(finish.y - start.y);
  int sy = (start.y < finish.y) ? 1 : -1;
  int err = ((dx > dy) ? dx : -dy) / 2;
  int e2;

  s_info = gbitmap_get_data_row_info(s_fb, start.y);
  while(true) {
    set_pixel(GPoint(start.x, start.y), color);
    if(start.x == finish.x && start.y == finish.y) {
      break;
    }
    e2 = err;
    if(e2 > -dx) {
      err -= dy;
      start.x += sx;
    }
    if(e2 < dy) {
      err += dx;
      start.y += sy;
      s_info = gbitmap_get_data_row_info(s_fb, start.y);
    }
  }
}

GBitmap* pge_isometric_begin(GContext *ctx) {
  s_fb = graphics_capture_frame_buffer(ctx);
  s_fb_data = gbitmap_get_data(s_fb);
  s_fb_size = gbitmap_get_bounds(s_fb).size;

  // Optionally further use the framebuffer GBitmap
  return s_fb;
}

void pge_isometric_finish(GContext *ctx) {
  if(s_fb) {
    graphics_release_frame_buffer(ctx, s_fb);

    s_fb = NULL;
    s_fb_data = NULL;
  }
}

void pge_isometric_set_enabled(bool b) {
  s_enabled = b;
}

void pge_isometric_set_projection_offset(GPoint offset) {
  s_projection_offset = offset;
}

GPoint pge_isometric_project(Vec3 input) {
  GPoint result;
  if(s_enabled) {
    result.x = s_projection_offset.x + (input.x - input.y);
    result.y = s_projection_offset.y + ((input.x / 2) + (input.y / 2)) - input.z;
  } else {
    result.x = input.x;
    result.y = input.y;
  }
  return result;
}

void pge_isometric_draw_rect(Vec3 origin, GSize size, GColor color) {
  // Top
  GPoint start = pge_isometric_project(origin);
  GPoint finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y, origin.z));
  bresenham_line(start, finish, color);

  // Right
  start = pge_isometric_project(Vec3(origin.x + size.w, origin.y, origin.z));
  finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, origin.z));
  bresenham_line(start, finish, color);

  // Bottom
  start = pge_isometric_project(Vec3(origin.x, origin.y + size.h, origin.z));
  finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, origin.z));
  bresenham_line(start, finish, color);

  // Left
  start = pge_isometric_project(origin);
  finish = pge_isometric_project(Vec3(origin.x, origin.y + size.h, origin.z));
  bresenham_line(start, finish, color);
}

void pge_isometric_fill_rect(Vec3 origin, GSize size, GColor color) {
  for(int i = 0; i < 2; i++) {
    for(int y = origin.y; y < origin.y + size.h; y++) {
      bresenham_line(
        pge_isometric_project(Vec3(origin.x, y, origin.z)), 
        pge_isometric_project(Vec3(origin.x + size.w, y, origin.z)),
        color
      );
    }
    origin.z--; // Draw twice to fill
  }
}

void pge_isometric_fill_box(Vec3 origin, GSize size, int z_height, GColor color) {
  // Draw only front for speed when PGE_ISOMETRIC_OPTIMIZE_FILL_BOX is defined
  int z = 0;
  for(z = origin.z; z < origin.z + z_height; z++) {
#if defined(PGE_ISOMETRIC_OPTIMIZE_FILL_BOX)
    // Right
    GPoint start = pge_isometric_project(Vec3(origin.x + size.w, origin.y, z));
    GPoint finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, z));
    bresenham_line(start, finish, color);

    // Bottom
    start = pge_isometric_project(Vec3(origin.x, origin.y + size.h, z));
    finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, z));
    bresenham_line(start, finish, color);
#else
    pge_isometric_draw_rect(Vec3(origin.x, origin.y, z), size, color);
#endif
  }

  // Fill in top
  pge_isometric_fill_rect(Vec3(origin.x, origin.y, (z_height > 0) ? z - 1 : z), size, color);
}

void pge_isometric_draw_box(Vec3 origin, GSize size, int z_height, GColor color) {
  // Bottom
  GPoint start = pge_isometric_project(Vec3(origin.x + size.w, origin.y, origin.z));
  GPoint finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, origin.z));
  bresenham_line(start, finish, color);
  start = pge_isometric_project(Vec3(origin.x, origin.y + size.h, origin.z));
  finish = pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, origin.z));
  bresenham_line(start, finish, color);

  // Top
  pge_isometric_draw_rect(Vec3(origin.x, origin.y, origin.z + z_height), size, color);

  // Sides
  bresenham_line(
    pge_isometric_project(Vec3(origin.x, origin.y + size.h, origin.z)), 
    pge_isometric_project(Vec3(origin.x, origin.y + size.h, origin.z + z_height)),
    color
  );
  bresenham_line(
    pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, origin.z)), 
    pge_isometric_project(Vec3(origin.x + size.w, origin.y + size.h, origin.z + z_height)),
    color
  );
  bresenham_line(
    pge_isometric_project(Vec3(origin.x + size.w, origin.y, origin.z)), 
    pge_isometric_project(Vec3(origin.x + size.w, origin.y, origin.z + z_height)),
    color
  );
}

void pge_isometric_draw_pixel(Vec3 point, GColor color) {
  s_info = gbitmap_get_data_row_info(s_fb, point.y);
  set_pixel(pge_isometric_project(point), color);
}

void pge_isometric_fill_textured_rect(Vec3 origin, GBitmap *texture) {
  // Map all bitmap pixels
  uint8_t *tex_data = gbitmap_get_data(texture);
  GSize tex_size = gbitmap_get_bounds(texture).size;
  uint8_t bytes_per_row = gbitmap_get_bytes_per_row(texture);
  
  for(int z = origin.z; z < origin.z + 2; z++) {
    for(int y = 0; y < tex_size.h; y++) {
      s_info = gbitmap_get_data_row_info(s_fb, origin.y);
      for(int x = 0; x < tex_size.w; x++) {
        uint8_t value = tex_data[(y * bytes_per_row) + x];
        set_pixel(pge_isometric_project(Vec3(origin.x + x, origin.y + y, z)), (GColor)value);
      }
    }
  }
}
