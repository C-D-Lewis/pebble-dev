#include <pebble.h>
#include "../lib/InverterLayerCompat.h"

static Window *window;
static TextLayer *s_time_label, *s_time_layer, *s_date_label, *s_date_layer, *s_prompt_label;

// TODO: This could just be a Layer that draws a white rectangle...
static InverterLayerCompat *s_prompt_layer;

static bool s_flash_visible = false;

/****************************** Other functions *********************************/

static TextLayer* make_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_load_custom_font(handle));
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}

/****************************** Time Lifecycle **********************************/

static void set_time(struct tm *t) {
  static char time_buffer[] = "XX:XX";
  strftime(time_buffer, sizeof("XX:XX"), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  text_layer_set_text(s_time_layer, time_buffer);

  static char date_buffer[] = "XX/XX/XXXX";
  strftime(date_buffer, sizeof("XX/XX/XXXX"), "%d/%m/%Y", t);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void tick_handler(struct tm *t, TimeUnits units_changed) {
  // Change time display
  set_time(t);

  // Do prompt flash
  if (s_flash_visible) {
    s_flash_visible = false;
    layer_remove_from_parent(inverter_layer_compat_get_layer(s_prompt_layer));
  } else {
    s_flash_visible = true;
    layer_add_child(window_get_root_layer(window), inverter_layer_compat_get_layer(s_prompt_layer));
  }
}

/***************************** Window Lifecycle *********************************/

static void window_load(Window *window) {
  // Font
  ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_LUCIDA_16);

  // Time 
  s_time_label = make_text_layer(GRect(5, 5, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_time_label, "C:\\>time /t");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_label));

  s_time_layer = make_text_layer(GRect(5, 24, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_time_layer, "NA:NA");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Date
  s_date_label = make_text_layer(GRect(5, 53, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_date_label, "C:\\>date /t");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_label));

  s_date_layer = make_text_layer(GRect(5, 70, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_date_layer, "XX/XX/XXXX");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Prompt
  s_prompt_label = make_text_layer(GRect(5, 99, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_prompt_label, "C:\\>");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_prompt_label));

  s_prompt_layer = inverter_layer_compat_create(GRect(45, 115, 12, 2), GColorWhite, GColorBlack);

  // Initial time display
  time_t temp = time(NULL);    
  struct tm *t = localtime(&temp);
  set_time(t);
}

static void window_unload(Window *window) {
  // Time
  text_layer_destroy(s_time_label);
  text_layer_destroy(s_time_layer);

  // Date
  text_layer_destroy(s_date_label);
  text_layer_destroy(s_date_layer);

  // Prompt
  text_layer_destroy(s_prompt_label);
  inverter_layer_compat_destroy(s_prompt_layer);

  window_destroy(window);
  window = NULL;
}

/******************************** App Lifecycle *********************************/

void main_window_push() {
  if (!window) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
    window_set_background_color(window, GColorBlack);
  }

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  window_stack_push(window, true);
}
