#include "splash_window.h"

#define INTERVAL_MS 150

static Window *s_window;
static BitmapLayer *s_logo_layer;
static Layer *s_spinner_layer;

static AppTimer *s_timer;
static GBitmap *s_logo_bitmap;
static int s_spinner_quarter;

/********************************** Spinner ***********************************/

static void spinner_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  const int spinner_size = 15;
  const int top = PBL_IF_ROUND_ELSE(130, 130);
  const int left_right = (bounds.size.w - spinner_size) / 2;
  GRect area = GRect(left_right, top, spinner_size, spinner_size);

  // Fill segment
  const int quarter_size = spinner_size / 2;
  GRect quarter_rect = GRectZero;
  switch(s_spinner_quarter) {
    case 0:
      quarter_rect = GRect(area.origin.x, area.origin.y, quarter_size, quarter_size);
      break;
    case 1:
      quarter_rect = GRect(area.origin.x + quarter_size, area.origin.y, quarter_size, quarter_size);
      break;
    case 2:
      quarter_rect = GRect(area.origin.x + quarter_size, area.origin.y + quarter_size, quarter_size, quarter_size);
      break;
    case 3:
      quarter_rect = GRect(area.origin.x, area.origin.y + quarter_size, quarter_size, quarter_size);
      break;
    default: break;
  } 
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, quarter_rect, 0, GCornerNone);
}

static void timer_handler(void *context) {
  s_spinner_quarter++;
  if(s_spinner_quarter == 4) {
    s_spinner_quarter = 0;
  }
  layer_mark_dirty(s_spinner_layer);

  s_timer = app_timer_register(INTERVAL_MS, timer_handler, NULL);
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_APP_ICON_LARGE);

  s_logo_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_logo_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));

  s_spinner_layer = layer_create(bounds);
  layer_set_update_proc(s_spinner_layer, spinner_update_proc);
  layer_add_child(window_layer, s_spinner_layer);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_logo_layer);
  gbitmap_destroy(s_logo_bitmap);

  layer_destroy(s_spinner_layer);

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void splash_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
  s_timer = app_timer_register(INTERVAL_MS, timer_handler, NULL);
}

void splash_window_pop() {
  window_stack_remove(s_window, true);
}