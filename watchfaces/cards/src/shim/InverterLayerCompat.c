#ifdef PBL_PLATFORM_BASALT
#include "InverterLayerCompat.h"

static GColor s_fg_color, s_bg_color;

static GColor get_pixel(uint8_t *fb_data, GSize fb_size, GPoint pixel) {
  if(pixel.x >= 0 && pixel.x < 144 && pixel.y >= 0 && pixel.y < 168) {
    return (GColor) { .argb = fb_data[(pixel.y * fb_size.w) + pixel.x] };
  } else {
    return GColorRed;
  }
}

static void set_pixel(uint8_t *fb_data, GSize fb_size, GPoint pixel, GColor color) {
  if(pixel.x >= 0 && pixel.x < 144 && pixel.y >= 0 && pixel.y < 168) {
    memset(&fb_data[(pixel.y * fb_size.w) + pixel.x], (uint8_t)color.argb, 1);
  }
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Use framebuffer to emulate inverstion
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  GSize size = gbitmap_get_bounds(fb).size;
  GRect frame = layer_get_frame(layer);
  uint8_t *fb_data = gbitmap_get_data(fb);

  for(int y = frame.origin.y; y < frame.origin.y + frame.size.h; y++) {
    for(int x = frame.origin.x; x < frame.origin.x + frame.size.w; x++) {
      if(gcolor_equal(get_pixel(fb_data, size, GPoint(x, y)), s_fg_color)) {
        // Invert foreground to background
        set_pixel(fb_data, size, GPoint(x, y), s_bg_color);
      } else if(gcolor_equal(get_pixel(fb_data, size, GPoint(x, y)), s_bg_color)) {
        // Invert background to foreground
        set_pixel(fb_data, size, GPoint(x, y), s_fg_color);
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

#endif