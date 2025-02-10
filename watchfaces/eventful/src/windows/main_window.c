#include "main_window.h"

static Window *s_window;
static TextLayer *s_time_layer, *s_unread_layer, *s_event_layer;
static BitmapLayer *s_icons_layer;

static GFont s_font;
static GBitmap *s_icons_bitmap;

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DS_DIGIB_60));

  int top = PBL_IF_ROUND_ELSE(5, -6);
  int left = PBL_IF_ROUND_ELSE(44, 32);
  s_time_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(top, 0, 0, 0)));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  top = PBL_IF_ROUND_ELSE(71, 61);
  s_unread_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(top, 0, 0, left)));
  text_layer_set_text_alignment(s_unread_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_unread_layer, GColorWhite);
  text_layer_set_background_color(s_unread_layer, GColorClear);
  text_layer_set_font(s_unread_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_unread_layer));

  top = PBL_IF_ROUND_ELSE(100, 90);
  int bottom = PBL_IF_ROUND_ELSE(27, 0);
  int right = PBL_IF_ROUND_ELSE(12, 0);
  s_event_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(top, right, bottom, left)));
  text_layer_set_text_alignment(s_event_layer, GTextAlignmentLeft);
  text_layer_set_text_color(s_event_layer, GColorWhite);
  text_layer_set_background_color(s_event_layer, GColorClear);
  text_layer_set_font(s_event_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_overflow_mode(s_event_layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(window_layer, text_layer_get_layer(s_event_layer));

  s_icons_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICONS);
  GSize icons_size = gbitmap_get_bounds(s_icons_bitmap).size;

  top = PBL_IF_ROUND_ELSE(78, 68);
  left = PBL_IF_ROUND_ELSE(14, 3);
  bottom = bounds.size.h - top - icons_size.h;
  right = bounds.size.w - left - icons_size.w;
  s_icons_layer = bitmap_layer_create(grect_inset(bounds, GEdgeInsets(top, right, bottom, left)));
  bitmap_layer_set_compositing_mode(s_icons_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_icons_layer, s_icons_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icons_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_unread_layer);
  text_layer_destroy(s_event_layer);
  fonts_unload_custom_font(s_font);

  bitmap_layer_destroy(s_icons_layer);
  gbitmap_destroy(s_icons_bitmap);

  window_destroy(s_window);
}

/************************************ API *************************************/

void main_window_push() {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void main_window_set_time(struct tm *tick_time) {
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), 
           (clock_is_24h_style() ? "%H:%M" : "%I:%M"), tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);
}

void main_window_set_unread_count(int count) {
  static char s_unread_buffer[8];
  snprintf(s_unread_buffer, sizeof(s_unread_buffer), "%d", count);
  text_layer_set_text(s_unread_layer, s_unread_buffer);
}

void main_window_set_next_event(char *event_text) {
  static char s_event_buffer[64];
  if(strcmp(event_text, "") == 0) {
    snprintf(s_event_buffer, sizeof(s_event_buffer), "None");
  } else {
    snprintf(s_event_buffer, sizeof(s_event_buffer), "%s", event_text);
  }
  text_layer_set_text(s_event_layer, s_event_buffer);
}
