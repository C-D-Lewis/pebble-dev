#include "message_window.h"

#define BRAID_H 14

#if defined(PBL_PLATFORM_EMERY)
  #define FONT_KEY FONT_KEY_GOTHIC_18
#else
  #define FONT_KEY FONT_KEY_GOTHIC_14
#endif

static Window *s_window;
static TextLayer *s_text_layer;
static BitmapLayer *s_image_layer;
static Layer *s_braid_layer;
static GBitmap *s_image_bitmap, *s_braid_bitmap;

static char *s_text_ptr;

static void braid_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  graphics_draw_bitmap_in_rect(
    ctx,
    s_braid_bitmap,
    GRect(0, 0, bounds.size.w, bounds.size.h)
  );
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  s_image_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MENU_ICON);
  s_braid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRAID);

  s_image_layer = bitmap_layer_create(GRect(0, scalable_y(2), scalable_x(100), 25));
  bitmap_layer_set_alignment(s_image_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  s_braid_layer = layer_create(GRect(0, scalable_y(17), DISPLAY_W, BRAID_H));
  layer_set_update_proc(s_braid_layer, braid_update_proc);
  layer_add_child(root_layer, s_braid_layer);

  s_text_layer = util_make_text_layer(
    scalable_grect(1, 26, 98, 100),
    fonts_get_system_font(FONT_KEY)
  );
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_layer, s_text_ptr);
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  bitmap_layer_destroy(s_image_layer);
  layer_destroy(s_braid_layer);

  gbitmap_destroy(s_image_bitmap);
  gbitmap_destroy(s_braid_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

void message_window_push(char *text) {
  s_text_ptr = text;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);
}
