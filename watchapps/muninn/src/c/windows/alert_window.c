#include "alert_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define FONT_KEY FONT_KEY_GOTHIC_24_BOLD
  #define TEXT_RECT GRect(0, 95, DISPLAY_W, DISPLAY_H)
#else
  #define FONT_KEY FONT_KEY_GOTHIC_18_BOLD
  #define TEXT_RECT GRect(2, 72, DISPLAY_W - 4, DISPLAY_H)
#endif

static Window *s_window;
static TextLayer *s_text_layer;
static BitmapLayer *s_image_layer;
static Layer *s_canvas_layer;
static GBitmap *s_image_bitmap, *s_braid_bitmap;

static uint32_t s_res_id;
static char *s_message;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_draw_bitmap_in_rect(ctx, s_braid_bitmap, GRect(0, 0, DISPLAY_W, 14));
  graphics_draw_bitmap_in_rect(ctx, s_braid_bitmap, GRect(0, DISPLAY_H - 14, DISPLAY_W, 14));
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_image_bitmap = gbitmap_create_with_resource(s_res_id);
  s_braid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRAID);

  s_image_layer = bitmap_layer_create(GRect(0, scalable_y(15), bounds.size.w, 48));
  bitmap_layer_set_alignment(s_image_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  const GFont font = fonts_get_system_font(FONT_KEY);

  s_text_layer = util_make_text_layer(TEXT_RECT, font);
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

  gbitmap_destroy(s_image_bitmap);
  gbitmap_destroy(s_braid_bitmap);

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
