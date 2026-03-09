#include "message_window.h"

#ifdef FEATURE_SYNC

#define QR_SIZE 100

static Window *s_window;
static TextLayer *s_title_layer, *s_id_layer;
static BitmapLayer *s_qr_layer;

static GBitmap *s_qr_bitmap;

static void window_load(Window *window) {
  AppState *app_state = data_get_app_state();

  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_title_layer = text_layer_create(
    GRect(0, scl_y_pp({.o = 10, .c = 40, .e = 60}), bounds.size.w, 50)
  );
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_font(s_title_layer, scl_get_font(SFI_Medium));
  text_layer_set_text(s_title_layer, "View full history:");
  layer_add_child(root_layer, text_layer_get_layer(s_title_layer));

  s_id_layer = text_layer_create(GRect(0, scl_y(780), bounds.size.w, 100));
  text_layer_set_text_alignment(s_id_layer, GTextAlignmentCenter);
  text_layer_set_font(s_id_layer, scl_get_font(SFI_LargeBold));
  layer_add_child(root_layer, text_layer_get_layer(s_id_layer));

  static char s_id_buff[7];
  snprintf(s_id_buff, sizeof(s_id_buff), "%s", app_state->upload_id);
  text_layer_set_text(s_id_layer, s_id_buff);

  s_qr_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QR_CODE);
  s_qr_layer = bitmap_layer_create(GRect(
    (bounds.size.w - QR_SIZE) / 2,
    (bounds.size.h - QR_SIZE) / 2,
    QR_SIZE,
    QR_SIZE
  ));
  bitmap_layer_set_bitmap(s_qr_layer, s_qr_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_qr_layer));
}

static void window_unload(Window *window) {
  gbitmap_destroy(s_qr_bitmap);
  bitmap_layer_destroy(s_qr_layer);
  text_layer_destroy(s_id_layer);
  text_layer_destroy(s_title_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void qr_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);
}

#endif
