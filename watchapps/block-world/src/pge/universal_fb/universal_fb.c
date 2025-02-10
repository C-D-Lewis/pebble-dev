/**
 * Universal set, get, and swap colors for Pebble SDK 3.0
 * Author: Chris Lewis
 * License: MIT
 */
 
#include "universal_fb.h"

/******************************* Compatibility ********************************/

// Provide GBitmapDataRowInfo API on 2.x
#if defined(PBL_SDK_2)
typedef struct {
  int min_x;
  int max_x;
  uint8_t *data;
} GBitmapDataRowInfo;

static GBitmapDataRowInfo gbitmap_get_data_row_info(GBitmap *bitmap, int y) {
  uint8_t *ptr = gbitmap_get_data(bitmap);
  int rsb = gbitmap_get_bytes_per_row(bitmap);
  ptr = &ptr[y * rsb];
  return (GBitmapDataRowInfo) {
    .min_x = 0, .max_x = 144, .data = ptr // Start of this row
  };
}
#endif

/********************************** Internal **********************************/

static bool byte_get_bit(uint8_t *byte, uint8_t bit) {
  return ((*byte) >> bit) & 1;
}

static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value) {
  *byte ^= (-value ^ *byte) & (1 << bit);
}

/************************************ API *************************************/

GColor universal_fb_get_pixel_color(GBitmap *fb, GPoint point) {
  GRect bounds = gbitmap_get_bounds(fb);
  GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, point.y);
  if(point.x > info.min_x && point.x < info.max_x
  && point.y > bounds.origin.y && point.y < bounds.origin.y + bounds.size.h) {
#if defined(PBL_COLOR)
    return (GColor){ .argb = info.data[point.x] };
#elif defined(PBL_BW)
    uint8_t byte = point.x / 8;
    uint8_t bit = point.x % 8; // fb: bwbb bbbb -> byte: 0000 0010
    return byte_get_bit(&info.data[byte], bit) ? GColorWhite : GColorBlack;
#endif
  } else {
    // Out of bounds
    return GColorClear;
  }
}

void universal_fb_set_pixel_color(GBitmap *fb, GPoint point, GColor color) {
  GRect bounds = gbitmap_get_bounds(fb);
  GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, point.y);
  if(point.x > info.min_x && point.x < info.max_x
  && point.y > bounds.origin.y && point.y < bounds.origin.y + bounds.size.h) {
#if defined(PBL_COLOR)
    memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
    uint8_t byte = point.x / 8;
    uint8_t bit = point.x % 8; // fb: bwbb bbbb -> byte: 0000 0010
    byte_set_bit(&info.data[byte], bit, color);
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
      if(gcolor_equal(universal_fb_get_pixel_color(fb, GPoint(x, y)), c1)) {
        // Replace c1 with c2
        universal_fb_set_pixel_color(fb, GPoint(x, y), c2);
      } else if(gcolor_equal(universal_fb_get_pixel_color(fb, GPoint(x, y)), c2)) {
        // Vice versa
        universal_fb_set_pixel_color(fb, GPoint(x, y), c1);
      }
    }
  }
}
