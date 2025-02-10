#include <pebble.h>
#include "SevenSegment.h"

//Animation values (ms)
#define ANIMATION_FRAME_INTERVAL 800
#define ANIM_DURATION 200
#define DELAY_FACTOR 50 

//Layout
#define SEGMENT_DIMS GSize(50, 10)
#define PROGRESS_BAR_HEIGHT 10

//Keys
#define KEY_INVERTED 0

//PS Keys
#define PERSIST_KEY_INVERTED 0

 //UI
static Window *main_window;
static InverterLayer 
  *progress_layer,
  *invert_layer;
static SevenSegment *segment;

//State
static bool animating;
static int last_time[4];
static int current_frame;

//Prototypes
static void animation_stopped_handler(Animation *animation, bool finished, void *context);
static void animate(Layer *layer, GRect start, GRect finish, int duration);
static void set_progress_bar_value(int perc);
static void update_prefs();

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *t = dict_read_first(iter);

  while(t != NULL)
  {
    switch(t->key)
    {
    case KEY_INVERTED:
      persist_write_bool(PERSIST_KEY_INVERTED, strcmp(t->value->cstring, "yes") == 0 ? true : false);

      update_prefs();
      break;
    }

    t = dict_read_next(iter);
  }
}
 
static void main_window_load(Window *window)
{
  segment = seven_segment_create(
    GPoint((144 - ((2 * SEGMENT_DIMS.h) + SEGMENT_DIMS.w)) / 2, (168 - ((3 * SEGMENT_DIMS.h) + (2 * SEGMENT_DIMS.w))) / 2),
    SEGMENT_DIMS, ANIM_DURATION, DELAY_FACTOR, window);

  progress_layer = inverter_layer_create(GRect(0, 168 - PROGRESS_BAR_HEIGHT, 0, PROGRESS_BAR_HEIGHT));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(progress_layer));

  invert_layer = inverter_layer_create(GRect(0, 0, 0, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(invert_layer));
}
 
static void main_window_unload(Window *window)
{
  seven_segment_destroy(segment);
  inverter_layer_destroy(progress_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed)
{
  int hours_12h = tick_time->tm_hour;
  if(hours_12h > 12)
  {
    hours_12h -= 12;
  }

  seven_segment_set_value(segment, hours_12h);
}

static void reset_handler(void *context)
{
  time_t temp = time(NULL); 
  struct tm *t = localtime(&temp);
  int hours = t->tm_hour;
  if(hours > 12)
  {
    hours -= 12;
  }
  seven_segment_set_value(segment, hours);
}

static void timer_handler(void *context)
{
  current_frame++;

  //0, 1, 2, 3
  if(current_frame < 4)
  {
    set_progress_bar_value((int)(((float)(current_frame + 1) / 4.0F) * 100.0F));
    seven_segment_set_value(segment, last_time[current_frame]);
    app_timer_register(ANIMATION_FRAME_INTERVAL, timer_handler, NULL);
  }
  else
  {
    //Can flick again!
    animating = false;

    //Reset hour display
    seven_segment_set_value(segment, -1);
    set_progress_bar_value(0);
    app_timer_register(ANIMATION_FRAME_INTERVAL, reset_handler, NULL);
  }
}

static void tap_handler(AccelAxisType axis, int32_t direction)
{
  if(animating == false)
  {
    animating = true;

    //Get time digits
    time_t temp = time(NULL); 
    struct tm *t = localtime(&temp);

    //Hour
    int hours = t->tm_hour;
    if(hours > 12)
    {
      hours -= 12;
    }
    if(hours < 10)
    {
      last_time[0] = 0;
      last_time[1] = hours;
    }
    else if(hours == 10)
    {
      last_time[0] = 1;
      last_time[1] = 0;
    }
    else if(hours == 11)
    {
      last_time[0] = 1;
      last_time[1] = 1;
    }
    else if(hours == 12)
    {
      last_time[0] = 1;
      last_time[1] = 2;
    }

    //Minutes
    if(t->tm_min < 10)
    {
      last_time[2] = 0;
      last_time[3] = t->tm_min;
    }
    else
    {
      last_time[2] = t->tm_min / 10;
      last_time[3] = t->tm_min % 10;
    }

    //Show all time digits, blanking first
    current_frame = -1;
    seven_segment_set_value(segment, current_frame);
    app_timer_register(ANIMATION_FRAME_INTERVAL, timer_handler, NULL);
  }
}

static void init()
{
  //Last time array
  for(int i = 0; i < 4; i++)
  {
    last_time[i] = 0;
  }

  //AppMessage
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  //Main Window
  main_window = window_create();
  WindowHandlers handlers = {
    .load = main_window_load,
    .unload = main_window_unload
  };
  window_set_window_handlers(main_window, (WindowHandlers) handlers);
  window_stack_push(main_window, true);

  //Change on the hour
  tick_timer_service_subscribe(HOUR_UNIT, tick_handler);

  //Show time with a shake
  accel_tap_service_subscribe(tap_handler);

  //Show current hour
  reset_handler(NULL);
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

/****************************** Other functions *******************************/

static void animation_stopped_handler(Animation *animation, bool finished, void *context)
{
  property_animation_destroy((PropertyAnimation*)animation);
}

static void animate(Layer *layer, GRect start, GRect finish, int duration)
{
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*)anim, duration);
  animation_set_curve((Animation*)anim, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)anim, (AnimationHandlers) {
    .stopped = animation_stopped_handler
  }, NULL);
  animation_schedule((Animation*)anim);
}

static void set_progress_bar_value(int perc)
{
  int width = (int)(((float)perc / 100.0F) * 144.0F);
  animate(inverter_layer_get_layer(progress_layer), 
    layer_get_frame(inverter_layer_get_layer(progress_layer)),
    GRect(0, 168 - PROGRESS_BAR_HEIGHT, width, PROGRESS_BAR_HEIGHT), ANIM_DURATION);
}

static void update_prefs()
{
  bool is_inverted = persist_read_bool(PERSIST_KEY_INVERTED);
  if(is_inverted == true)
  {
    //Invert the whole screen
    layer_set_frame(inverter_layer_get_layer(invert_layer), GRect(0, 0, 144, 168));
  }
  else
  {
    layer_set_frame(inverter_layer_get_layer(invert_layer), GRect(0, 0, 0, 0));
  }
}