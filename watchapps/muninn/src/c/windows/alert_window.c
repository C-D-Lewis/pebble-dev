#include "alert_window.h"

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
  #define BRAID_H 18
#else
  #define BRAID_H 14
#endif

static Window *s_window;
static TextLayer *s_text_layer;
static BitmapLayer *s_image_layer;
static Layer *s_canvas_layer;
static GBitmap *s_image_bitmap;

static uint32_t s_res_id;
static char *s_message;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect top_braid_rect = GRect(0, 0, PS_DISP_W, BRAID_H);
  GRect bottom_braid_rect = GRect(0, PS_DISP_H - BRAID_H, PS_DISP_W, BRAID_H);

  util_draw_braid(ctx, top_braid_rect);
  util_draw_braid(ctx, bottom_braid_rect);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_image_bitmap = bitmaps_get(s_res_id);
  s_image_layer = bitmap_layer_create(scalable_grect(0, 130, 1000, 300));
  bitmap_layer_set_alignment(s_image_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  s_text_layer = util_make_text_layer(
    scalable_grect(0, 430, 1000, 1000),
    scalable_get_font(SFI_Medium)
  );
  text_layer_set_text(s_text_layer, s_message);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  bitmap_layer_destroy(s_image_layer);
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void timer_callback(void *data) {
  window_stack_pop_all(true);
}

void alert_window_push(uint32_t res_id, char *message, bool do_vibe, bool do_dismiss) {
  s_res_id = res_id;
  s_message = message;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  if (do_dismiss) app_timer_register(3000, timer_callback, NULL);
  if (do_vibe) vibes_short_pulse();
}
