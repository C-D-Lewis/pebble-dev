#include <pebble.h>
#include "cl_progressbar.h"

#define DELAY 33
 
static Window* window;
static ProgressBarLayer *pb_layer;
static AppTimer *timer;

static void timer_callback(void *context)
{
  //Add and/or wrap
  int new_progress = progressbar_layer_get_progress(pb_layer) + 1;
  if(new_progress > 100)
  {
    new_progress = 0;
  }

  //Set new progress value
  progressbar_layer_set_progress(pb_layer, new_progress);

  //Repeat
  timer = app_timer_register(DELAY, (AppTimerCallback)timer_callback, NULL);
}
 
static void window_load(Window *window)
{
  //Add a new ProgressBarLayer
  pb_layer = progressbar_layer_create(GRect(5, 5, 134, 15));
  layer_add_child(window_get_root_layer(window), progressbar_layer_get_layer(pb_layer));

  //Register progression
  timer = app_timer_register(DELAY, (AppTimerCallback)timer_callback, NULL);
}
 
static void window_unload(Window *window)
{
  //Destroy ProgressBarLayer
  progressbar_layer_destroy(pb_layer);
}
 
static void init()
{
  //Create Window
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
  //Destroy Window
  window_destroy(window);
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}
