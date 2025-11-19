#include "alert_window.h"

static Window *s_window;
static TextLayer *s_text_layer;
static BitmapLayer *s_image_layer;
static GBitmap *s_image_bitmap;

static uint32_t s_res_id;
static char *s_message;

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_image_bitmap = gbitmap_create_with_resource(s_res_id);

  s_image_layer = bitmap_layer_create(GRect(0, 20, bounds.size.w, 64));
  bitmap_layer_set_alignment(s_image_layer, GAlignCenter);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  s_text_layer = util_make_text_layer(
    GRect(5, 90, DISPLAY_W - 10, 100),
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD)
  );
  text_layer_set_text(s_text_layer, s_message);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  bitmap_layer_destroy(s_image_layer);

  gbitmap_destroy(s_image_bitmap);

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
