#include <pebble.h>
#include "include/cl_util.h"
 
static Window* window;
static TextLayer
  *hour_layer,
  *minute_layer;
 
static void window_load(Window *window)
{
  window_set_background_color(window, GColorBlack);

  ResHandle handle = resource_get_handle(RESOURCE_ID_FONT_41);
  GFont *font = fonts_load_custom_font(handle);
  hour_layer = cl_text_layer_create(GRect(0, 59, 110, 50), GColorWhite, GColorClear, true, font, NULL, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hour_layer));

  ResHandle handle_2 = resource_get_handle(RESOURCE_ID_FONT_24);
  GFont *font_2 = fonts_load_custom_font(handle_2);
  minute_layer = cl_text_layer_create(GRect(100, 60, 38, 50), GColorWhite, GColorClear, true, font_2, NULL, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minute_layer));
}
 
static void window_unload(Window *window)
{
  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_layer);
}

static void tick_handler(struct tm* tick_time, TimeUnits changed)
{
  int hour = tick_time->tm_hour;
  if(hour > 12)
  {
    hour = hour - 12;
  }

  //TEST
  // hour = 11;

  switch(hour)
  {
  case 0:
    text_layer_set_text(hour_layer, "zero");
    break;
  case 1:
    text_layer_set_text(hour_layer, "one");
    break;
  case 2:
    text_layer_set_text(hour_layer, "two");
    break;
  case 3:
    text_layer_set_text(hour_layer, "three");
    break;
  case 4:
    text_layer_set_text(hour_layer, "four");
    break;
  case 5:
    text_layer_set_text(hour_layer, "five");
    break;
  case 6:
    text_layer_set_text(hour_layer, "six");
    break;
  case 7:
    text_layer_set_text(hour_layer, "seven");
    break;
  case 8:
    text_layer_set_text(hour_layer, "eight");
    break;
  case 9:
    text_layer_set_text(hour_layer, "nine");
    break;
  case 10:
    text_layer_set_text(hour_layer, "ten");
    break;
  case 11:
    text_layer_set_text(hour_layer, "eleven");
    break;
  case 12:
    text_layer_set_text(hour_layer, "twelve");
    break;
  }

  static char buffer[] = "00";
  strftime(buffer, sizeof("00"), "%M", tick_time);
  text_layer_set_text(minute_layer, buffer);
}
 
static void init()
{
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, (WindowHandlers) handlers);
  window_stack_push(window, true);
}
 
static void deinit()
{
  window_destroy(window);
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}