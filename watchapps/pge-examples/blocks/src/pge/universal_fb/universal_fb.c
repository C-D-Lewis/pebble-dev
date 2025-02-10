/**
 * Universal set, get, and swap colors for Pebble SDK 3.0
 * Author: Chris Lewis
 * License: MIT
 */
 
#include "universal_fb.h"

static bool byte_get_bit(uint8_t *byte, uint8_t bit) {
  return ((*byte) >> bit) & 1;
}

static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value) {
  *byte ^= (-value ^ *byte) & (1 << bit);
}

/************************************ API *************************************/

GColor universal_fb_get_pixel_color(GBitmapDataRowInfo info, GRect bounds, GPoint point) {
  if(point.x > info.min_x && point.x < info.max_x
  && point.y > bounds.origin.y && point.y < bounds.origin.y + bounds.size.h) {
#if defined(PBL_COLOR)
    return (GColor){ .argb = info.data[point.x] };
#elif defined(PBL_BW)
    uint8_t byte = point.x / 8;
    uint8_t bit = point.x % 8; 
    return byte_get_bit(&info.data[byte], bit) ? GColorWhite : GColorBlack;
#endif
  } else {
    // Out of bounds
    return GColorClear;
  }
}

void universal_fb_set_pixel_color(GBitmapDataRowInfo info, GRect bounds, GPoint point, GColor color) {
  if(point.x > info.min_x && point.x < info.max_x
  && point.y > bounds.origin.y && point.y < bounds.origin.y + bounds.size.h) {
#if defined(PBL_COLOR)
    memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
    uint8_t byte = point.x / 8;
    uint8_t bit = point.x % 8; 
    byte_set_bit(&info.data[byte], bit, gcolor_equal(color, GColorWhite) ? 1 : 0);
#endif
  } else {
    // Out of bounds
    return;
  }
}

void universal_fb_swap_colors(GBitmap *fb, GRect bounds, GColor c1, GColor c2) {
  for(int y = bounds.origin.y; y < bounds.origin.y + bounds.size.h; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
    for(int x = bounds.origin.x; x < bounds.origin.x + bounds.size.w; x++) {
      GPoint p = GPoint(x, y);
      if(gcolor_equal(universal_fb_get_pixel_color(info, bounds, p), c1)) {
        universal_fb_set_pixel_color(info, bounds, p, c2);
      } else if(gcolor_equal(universal_fb_get_pixel_color(info, bounds, p), c2)) {
        universal_fb_set_pixel_color(info, bounds, p, c1);
      }
    }
  }
}
