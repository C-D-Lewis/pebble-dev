#include "main_window.h"

static Window *s_window;
static BitmapLayer *s_image_layer;
static TextLayer
  *s_status_label_layer,
  *s_status_value_layer;
static GBitmap *s_image_bitmap;

static void update_data() {
  gbitmap_destroy(s_image_bitmap);

  bool is_enabled = data_get_wakeup_id() != DATA_EMPTY;
  s_image_bitmap = gbitmap_create_with_resource(is_enabled ? RESOURCE_ID_AWAKE : RESOURCE_ID_ASLEEP);

  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool should_enable = data_get_wakeup_id() == DATA_EMPTY;

  if (should_enable) {
    data_prepare();
    data_sample_now();
    wakeup_schedule_next();
  } else {
    wakeup_unschedule();
  }

  update_data();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  GFont sys_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GFont sys_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  s_image_layer = bitmap_layer_create(GRect(5, 5, 64, 64));
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  s_status_label_layer = util_make_text_layer(GRect(72, 10, WIDTH - 75, 100), sys_18);
  text_layer_set_text(s_status_label_layer, "Muninn is");
  layer_add_child(root_layer, text_layer_get_layer(s_status_label_layer));

  s_status_value_layer = util_make_text_layer(GRect(71, 22, WIDTH - 75, 100), sys_28);
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  update_data();
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_image_layer);
  text_layer_destroy(s_status_label_layer);
  text_layer_destroy(s_status_value_layer);

  gbitmap_destroy(s_image_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(s_window, click_config_provider);
  }

  window_stack_push(s_window, true);
}
