#include "message_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define BRAID_H 18
#else
  #define BRAID_H 14
#endif

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_text_layer;
static Layer *s_braid_layer;
#if !defined(PBL_PLATFORM_APLITE)
static BitmapLayer *s_image_layer;
static GBitmap *s_image_bitmap;
#endif

static char *s_text_ptr;

static void braid_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  GRect braid_rect = GRect(0, 0, bounds.size.w, bounds.size.h);
  util_draw_braid(ctx, braid_rect);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

#if !defined(PBL_PLATFORM_APLITE)
  s_image_bitmap = bitmaps_create(RESOURCE_ID_AWAKE_HEAD);
  s_image_layer = bitmap_layer_create(
    scalable_grect_pp(
      GRect(0, 10, 1000, 150),
      GRect(0, 20, 1000, 150)
    )
  );
  bitmap_layer_set_alignment(s_image_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));
#endif

#if !defined(PBL_PLATFORM_APLITE)
  const int braid_y = scalable_y(170);
#else
  const int braid_y = 0;
#endif
  s_braid_layer = layer_create(GRect(0, braid_y, DISPLAY_W, BRAID_H));
  layer_set_update_proc(s_braid_layer, braid_update_proc);
  layer_add_child(root_layer, s_braid_layer);

  // Code from devsite to try and fit text inside a TextLayer inside a ScrollLayer
  GRect shrinking_rect = GRect(5, 0, bounds.size.w - 10, 2000);
  GSize text_size = graphics_text_layout_get_content_size(
    s_text_ptr,
    scalable_get_font(SFI_Medium), 
    shrinking_rect,
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft
  );
  text_size.h += 10;
  GRect text_bounds = bounds;
  text_bounds.origin.x += 5;
  text_bounds.size.w -= 10;
  text_bounds.size.h = text_size.h;

  s_text_layer = util_make_text_layer(text_bounds, scalable_get_font(SFI_Medium));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  text_layer_set_text(s_text_layer, s_text_ptr);
  
  const int scroll_y = braid_y + BRAID_H;
  s_scroll_layer = scroll_layer_create(
    GRect(0, scroll_y, bounds.size.w, bounds.size.h - scroll_y)
  );
  scroll_layer_set_content_size(s_scroll_layer, text_size);
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(root_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(s_text_layer);
#if !defined(PBL_PLATFORM_APLITE)
  bitmap_layer_destroy(s_image_layer);
#endif
  layer_destroy(s_braid_layer);

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
