#include "InverterLayerCompat.h"

static bool byte_get_bit(uint8_t *byte, uint8_t bit) {
  return ((*byte) >> bit) & 1;
}

static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value) {
  *byte ^= (-value ^ *byte) & (1 << bit);
}

static GColor get_pixel_color(GBitmapDataRowInfo info, GPoint point) {
#if defined(PBL_COLOR)
  // Read the single byte color pixel
  return (GColor){ .argb = info.data[point.x] };
#elif defined(PBL_BW)
  // Read the single bit of the correct byte
  uint8_t byte = point.x / 8;
  uint8_t bit = point.x % 8; 
  return byte_get_bit(&info.data[byte], bit) ? GColorWhite : GColorBlack;
#endif
}

static void set_pixel_color(GBitmapDataRowInfo info, GPoint point, 
                                                                GColor color) {
#if defined(PBL_COLOR)
  // Write the pixel's byte color
  memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
  // Find the correct byte, then set the appropriate bit
  uint8_t byte = point.x / 8;
  uint8_t bit = point.x % 8; 
  byte_set_bit(&info.data[byte], bit, gcolor_equal(color, GColorWhite) ? 1 : 0);
#endif
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Use framebuffer to emulate inversion
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  GSize size = gbitmap_get_bounds(fb).size;
  GRect frame = layer_get_frame(layer);
  InverterLayerCompatInfo ilc_info = *(InverterLayerCompatInfo*)layer_get_data(layer);
  
  uint8_t *fb_data = gbitmap_get_data(fb);

  // Iterate over the whole frame but only within the layer's bounds
  for (int y = frame.origin.y; y < frame.origin.y + frame.size.h; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);

    for (int x = frame.origin.x; x < frame.origin.x + frame.size.w; x++) {
      GColor pixel_color = get_pixel_color(info, GPoint(x, y));
      
      if (gcolor_equal(pixel_color, ilc_info.fg_color)) {
        // Invert foreground to background
        set_pixel_color(info, GPoint(x, y), ilc_info.bg_color);
      } else if (gcolor_equal(pixel_color, ilc_info.bg_color)) {
        // Invert background to foreground
        set_pixel_color(info, GPoint(x, y), ilc_info.fg_color);
      }
    }
  }

  // Finally
  graphics_release_frame_buffer(ctx, fb);
}

InverterLayerCompat *inverter_layer_compat_create(GRect bounds, GColor fg, GColor bg) {
  InverterLayerCompat *this = (InverterLayerCompat*)malloc(sizeof(InverterLayerCompat));
  this->layer = layer_create_with_data(bounds, sizeof(InverterLayerCompatInfo));
  InverterLayerCompatInfo *ilc_info = (InverterLayerCompatInfo*)layer_get_data(this->layer);
  ilc_info->fg_color = fg;
  ilc_info->bg_color = bg;
  layer_set_update_proc(this->layer, layer_update_proc);

  return this;
}

void inverter_layer_compat_destroy(InverterLayerCompat *this) {
  layer_destroy(this->layer);
  free(this);
}

Layer* inverter_layer_compat_get_layer(InverterLayerCompat *this) {
  return this->layer;
}
