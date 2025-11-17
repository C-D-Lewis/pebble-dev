#include "about_window.h"

static Window *s_window;
static TextLayer *s_text_layer;
static BitmapLayer *s_image_layer, *s_braid_layer;
static GBitmap *s_image_bitmap, *s_braid_bitmap;

const char* about_text = "Odin tasked Muninn with memory of the land...\n\nHe wakes every 12 hours to note the battery level.\n\nOver time, he will provide you with battery wisdom.";

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_image_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MENU_ICON);
  s_braid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRAID);

  s_image_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, 25));
  bitmap_layer_set_alignment(s_image_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  s_braid_layer = bitmap_layer_create(GRect(0, 30, WIDTH, 14));
  bitmap_layer_set_compositing_mode(s_braid_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_braid_layer, s_braid_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_braid_layer));

  s_text_layer = util_make_text_layer(GRect(5, 48, WIDTH - 10, HEIGHT), fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_text_layer, about_text);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  bitmap_layer_destroy(s_image_layer);
  bitmap_layer_destroy(s_braid_layer);

  gbitmap_destroy(s_image_bitmap);
  gbitmap_destroy(s_braid_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

void about_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);
}
