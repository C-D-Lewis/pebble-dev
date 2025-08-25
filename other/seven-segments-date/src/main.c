#include <pebble.h>
#include "SevenSegment.h"

//Animation values (ms)
#define ANIMATION_FRAME_INTERVAL 800
#define ANIM_DURATION            200
#define DELAY_FACTOR             50

//Layout
#define TIME_DIMS GSize(30, 6)
#define DATE_DIMS GSize(15, 3)

 //UI
static Window *main_window;
static InverterLayer *invert_layer;
static SevenSegment
  *time_segments[4],
  *date_segments[4];

//State
static int 
  time_digits[4],
  date_digits[4];
static int last_animated_digit;;
static bool animating;

static void anim_frame_handler(void *context)
{
  if(last_animated_digit < 8)
  {
    if(last_animated_digit < 4)
    {
      seven_segment_set_value(time_segments[last_animated_digit], time_digits[last_animated_digit]);
    }
    else
    {
      seven_segment_set_value(date_segments[last_animated_digit - 4], date_digits[last_animated_digit - 4]); 
    }

    last_animated_digit++;
    app_timer_register(3 * DELAY_FACTOR, anim_frame_handler, NULL);
  }
  else
  {
    animating = false;
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction)
{
  //Animate?
  if(animating == false)
  {
    animating = true;
    last_animated_digit = 0;

    //Clear
    for(int i = 0; i < 4; i++)
    {
      seven_segment_set_value(time_segments[i], -1);
      seven_segment_set_value(date_segments[i], -1);
    }

    app_timer_register(1000, anim_frame_handler, NULL);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed)
{
  //Hour
  int hours = tick_time->tm_hour;
  if(hours < 10)
  {
    time_digits[0] = 0;
    time_digits[1] = hours;
  }
  else
  {
    time_digits[0] = hours / 10;
    time_digits[1] = hours % 10;
  }

  //Minutes
  int minutes = tick_time->tm_min;
  if(minutes < 10)
  {
    time_digits[2] = 0;
    time_digits[3] = minutes;
  }
  else
  {
    time_digits[2] = minutes / 10;
    time_digits[3] = minutes % 10;
  }

  //Show date DD MM
  int day = tick_time->tm_mday;
  if(day < 10)
  {
    date_digits[0] = 0;
    date_digits[1] = day;
  }
  else
  {
    date_digits[0] = day / 10;
    date_digits[1] = day % 10;
  }

  int month = tick_time->tm_mon + 1; //Don't want month 0!
  if(month < 10)
  {
    date_digits[2] = 0;
    date_digits[3] = month;
  }
  else
  {
    date_digits[2] = 1;
    date_digits[3] = month % 10;
  }

  if(animating == false)
  {
    //Update those that have changed
    for(int i = 0; i < 4; i++)
    {
      seven_segment_set_value(time_segments[i], time_digits[i]);
      seven_segment_set_value(date_segments[i], date_digits[i]);
    }
  }
}
 
static void main_window_load(Window *window)
{
  //Segment arrays
  time_segments[0] = seven_segment_create(GPoint(0, 2), 
    TIME_DIMS, ANIM_DURATION, DELAY_FACTOR, window);
  time_segments[1] = seven_segment_create(GPoint(0 + (3 * TIME_DIMS.h) + TIME_DIMS.w, 2), 
    TIME_DIMS, ANIM_DURATION, DELAY_FACTOR, window);
  time_segments[2] = seven_segment_create(GPoint(0, 2 + (2 * TIME_DIMS.w) + (4 * TIME_DIMS.h)), 
    TIME_DIMS, ANIM_DURATION, DELAY_FACTOR, window);
  time_segments[3] = seven_segment_create(GPoint(0 + (3 * TIME_DIMS.h) + TIME_DIMS.w, 2 + (2 * TIME_DIMS.w) + (4 * TIME_DIMS.h)), 
    TIME_DIMS, ANIM_DURATION, DELAY_FACTOR, window);

  date_segments[0] = seven_segment_create(GPoint(98, (4 * DATE_DIMS.h) + (2 * DATE_DIMS.w) - 1), 
    DATE_DIMS, ANIM_DURATION, DELAY_FACTOR, window);
  date_segments[1] = seven_segment_create(GPoint(98 + (3 * DATE_DIMS.h) + DATE_DIMS.w, (4 * DATE_DIMS.h) + (2 * DATE_DIMS.w) - 1), 
    DATE_DIMS, ANIM_DURATION, DELAY_FACTOR, window);
  date_segments[2] = seven_segment_create(GPoint(98, 2 * ((4 * DATE_DIMS.h) + (2 * DATE_DIMS.w)) + 1), 
    DATE_DIMS, ANIM_DURATION, DELAY_FACTOR, window);
  date_segments[3] = seven_segment_create(GPoint(98 + (3 * DATE_DIMS.h) + DATE_DIMS.w, 2 * ((4 * DATE_DIMS.h) + (2 * DATE_DIMS.w)) + 1), 
    DATE_DIMS, ANIM_DURATION, DELAY_FACTOR, window);

  invert_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(invert_layer));

  tap_handler(ACCEL_AXIS_X, 1);
}
 
static void main_window_unload(Window *window)
{
  for(int i = 0; i < 4; i++)
  {
    if(time_segments[i] != NULL)
    {
      seven_segment_destroy(time_segments[i]);
    }
    if(date_segments[i] != NULL)
    {
      seven_segment_destroy(date_segments[i]);
    }
  }

  inverter_layer_destroy(invert_layer);
}

static void init()
{
  //Time arrays
  for(int i = 0; i < 4; i++)
  {
    time_digits[i] = 0;
    date_digits[i] = 0;
  }

  //Main Window
  main_window = window_create();
  WindowHandlers handlers = {
    .load = main_window_load,
    .unload = main_window_unload
  };
  window_set_window_handlers(main_window, (WindowHandlers) handlers);
  window_stack_push(main_window, true);

  //Shake to animate
  accel_tap_service_subscribe(tap_handler);

  //Change on the hour
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
 
static void deinit()
{
  window_destroy(main_window);
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}
