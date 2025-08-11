#include "InverterLayerCompat.h"

static GColor s_fg_color, s_bg_color;

static bool byte_get_bit(uint8_t *byte, uint8_t bit) {
  return ((*byte) >> bit) & 1;
}

static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value) {
  *byte ^= (-value ^ *byte) & (1 << bit);
}

static GColor get_pixel_color(uint8_t *fb_data, GSize fb_size, GPoint layer_origin, GPoint pixel) {
  int offset = (pixel.y * fb_size.w) + pixel.x;

  // Need to consider the layer origin
  pixel.x += layer_origin.x;
  pixel.y += layer_origin.y;

  if (pixel.x >= 0 && pixel.x < fb_size.w && pixel.y >= 0 && pixel.y < fb_size.h) {
#if defined(PBL_COLOR)
    // Read whole byte
    return (GColor) { .argb = fb_data[offset] };
#else
    // Read the bit
    int byte_index = offset / 8;
    uint8_t bit_index = offset % 8;
    if (pixel.x == 100 && pixel.y == 100) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "x,y: %d,%d, offset: %d, byte_index: %d, bit_index: %d", pixel.x, pixel.y, offset, byte_index, bit_index);
    }
    return byte_get_bit(&fb_data[byte_index], bit_index) ? GColorWhite : GColorBlack;
#endif
  } else {
    return PBL_IF_COLOR_ELSE(GColorRed, GColorBlack); // Default color if out of bounds
  }
}

static void set_pixel_color(uint8_t *fb_data, GSize fb_size, GPoint pixel, GColor color) {
  int offset = (pixel.y * fb_size.w) + pixel.x;

  if (pixel.x >= 0 && pixel.x < fb_size.w && pixel.y >= 0 && pixel.y < fb_size.h) {
#if defined(PBL_COLOR)
    // Write whole byte
    memset(&fb_data[offset], (uint8_t)color.argb, 1);
#else
    // Write the bit
    int byte_index = offset / 8;
    uint8_t bit_index = offset % 8;
    byte_set_bit(&fb_data[byte_index], bit_index, gcolor_equal(color, GColorWhite) ? 1 : 0);
#endif
  }
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Use framebuffer to emulate inversion
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  GSize size = gbitmap_get_bounds(fb).size;
  GRect frame = layer_get_frame(layer);
  
  uint8_t *fb_data = gbitmap_get_data(fb);

  for (int y = frame.origin.y; y < frame.origin.y + frame.size.h; y++) {
    for (int x = frame.origin.x; x < frame.origin.x + frame.size.w; x++) {
      GColor pixel_color = get_pixel_color(fb_data, size, frame.origin, GPoint(x, y));
      if (gcolor_equal(pixel_color, s_fg_color)) {
        // Invert foreground to background
        set_pixel_color(fb_data, size, GPoint(x, y), s_bg_color);
      } else if (gcolor_equal(pixel_color, s_bg_color)) {
        // Invert background to foreground
        set_pixel_color(fb_data, size, GPoint(x, y), s_fg_color);
      }
    }
  }

  // Finally
  graphics_release_frame_buffer(ctx, fb);
}

InverterLayerCompat *inverter_layer_compat_create(GRect bounds) {
  InverterLayerCompat *this = (InverterLayerCompat*)malloc(sizeof(InverterLayerCompat));
  this->layer = layer_create(bounds);
  layer_set_update_proc(this->layer, layer_update_proc);

  return this;
}

void inverter_layer_compat_set_colors(GColor fg, GColor bg) {
  s_fg_color = fg;
  s_bg_color = bg;
}

void inverter_layer_compat_destroy(InverterLayerCompat *this) {
  layer_destroy(this->layer);
  free(this);
}

Layer* inverter_layer_compat_get_layer(InverterLayerCompat *this) {
  return this->layer;
}
