#if defined(PBL_COLOR)
#include "image_window.h"

#define NUM_COLORS 64

static Window *s_window;
static TextLayer *s_label_layer;
static Layer *s_thumb_layer;

static int s_index;
static int s_palette[NUM_COLORS]; // Index is the color value
static bool s_palette_found;

/********************************* Thumbnail **********************************/

static void get_image() {
  comm_get_thumbnail(s_index);
  layer_mark_dirty(s_thumb_layer);
}

static void generate_palette_info(GBitmap *bitmap) {
  for(int i = 0; i < NUM_COLORS; i++) {
    s_palette[i] = 0;
  }

  GSize bitmap_size = gbitmap_get_bounds(bitmap).size;
  for(int y = 0; y < bitmap_size.h; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(bitmap, y);
    for(int x = info.min_x; x < info.max_x; x++) {
      uint8_t val = info.data[x] & 0b00111111;  // 0 - 64 only
      s_palette[val]++;
    }
  }
}

static GColor get_most_common_color() {
#if defined(PBL_COLOR)
  GBitmap *bitmap = data_get_thumbnail();
  if(!bitmap) {
    return GColorBlack;
  }

  generate_palette_info(bitmap);

  int highest_val = 0;
  int highest_index = 0;
  for(int i = 0; i < NUM_COLORS; i++) {
    if(s_palette[i] > highest_val) {
      highest_index = i;
      highest_val = s_palette[i];
    }
  }
  return (GColor){ .argb = (highest_index | 0b11000000) };
#else
  return GColorBlack;
#endif
}

static void thumb_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  GBitmap *bitmap = data_get_thumbnail();
  if(!bitmap) {
    graphics_context_set_text_color(ctx, GColorWhite);
    if(data_get_thumbnail_progress() >= 0) {
      static char s_buff[32];
      snprintf(s_buff, sizeof(s_buff), "\n%d%%", data_get_thumbnail_progress());
      graphics_draw_text(ctx, s_buff, fonts_get_system_font(FONT_KEY_GOTHIC_24), bounds,
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    } else {
      graphics_draw_text(ctx, "No image for\nthis story", fonts_get_system_font(FONT_KEY_GOTHIC_24), bounds,
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    return;
  }
  
  GSize bitmap_size = gbitmap_get_bounds(bitmap).size;

  // Color stretch
  GColor color = get_most_common_color();
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, bounds, 0, 0);

  const int x_margin = (bounds.size.w - bitmap_size.w) / 2;
  graphics_draw_bitmap_in_rect(ctx, bitmap, grect_inset(bounds, GEdgeInsets(0, x_margin)));
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  GEdgeInsets thumb_insets = GEdgeInsets((bounds.size.h - THUMBNAIL_HEIGHT) / 2, 0);
  s_thumb_layer = layer_create(grect_inset(bounds, thumb_insets));
  layer_set_update_proc(s_thumb_layer, thumb_update_proc);
  layer_add_child(window_layer, s_thumb_layer);

  const int margin_y = PBL_IF_ROUND_ELSE(18, 12);
  s_label_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(margin_y, 0)));
  text_layer_set_text(s_label_layer, "Image");
  text_layer_set_text_color(s_label_layer, GColorWhite);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void window_unload(Window *window) {
  layer_destroy(s_thumb_layer);
  text_layer_destroy(s_label_layer);
  window_destroy(s_window);
  s_window = NULL;
}
#endif

/************************************ API *************************************/

void image_window_push(int index) {
#if defined(PBL_COLOR)
  s_palette_found = false;

  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  s_index = index;
  get_image();
#endif
}


// Stub instead of lots of ifdefs
void image_window_redraw_thumbnail() {
#if defined(PBL_COLOR)
  if(s_thumb_layer) {
    layer_mark_dirty(s_thumb_layer);

    GColor color = get_most_common_color();
    window_set_background_color(s_window, color);
    text_layer_set_text_color(s_label_layer, gcolor_legible_over(color));
  }
#endif
}
