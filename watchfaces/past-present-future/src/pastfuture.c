#include <pebble.h>
#include "cl_util.h"
#define INVERT true //used for bulding inverted version
#define TWELVE_HOUR true //used for building 12-hour version

static Window *window;
static BitmapLayer *past_bitmap_layer, *future_bitmap_layer, *mask_layer, *present_bitmap_layer;
static GBitmap *past_gbitmap, *future_gbitmap, *mask_gbitmap, *present_gbitmap;
static TextLayer *hour_layer, *minute_layer;
static char *hour_buff, *minute_buff;
#if TWELVE_HOUR
static char ampm_text[] = "xx";
#endif
#if INVERT
static InverterLayer *invert_layer;
endif

static void window_load(Window *window) 
{
  past_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_PAST);
  past_bitmap_layer = bitmap_layer_create(GRect(-3, 0, 33, 168));
  bitmap_layer_set_bitmap(past_bitmap_layer, past_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(past_bitmap_layer));

  present_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_PRESENT);
  present_bitmap_layer = bitmap_layer_create(GRect(55, 0, 33, 168));
  bitmap_layer_set_bitmap(present_bitmap_layer, present_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(present_bitmap_layer));

  future_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_FUTURE);
  future_bitmap_layer = bitmap_layer_create(GRect(112, 0, 33, 168));
  bitmap_layer_set_bitmap(future_bitmap_layer, future_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(future_bitmap_layer));

  hour_layer = cl_init_text_layer(GRect(0, 5, 144, 60), GColorBlack, GColorClear, true, RESOURCE_ID_FONT_MEGAFONT_22, 0, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hour_layer));

  minute_layer = cl_init_text_layer(GRect(0, 25, 144, 60), GColorBlack, GColorClear, true, RESOURCE_ID_FONT_MEGAFONT_22, 0, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minute_layer));
  
  #if TWELVE_HOUR
  ampm_layer = cl_init_text_layer(GRect(0, 141, 144, 60), GColorBlack, GColorClear, true, RESOURCE_ID_FONT_MEGAFONT_22, 0, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(ampm_layer));
  #endif

  mask_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_MASK);
  mask_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(mask_layer, GCompOpOr);
  bitmap_layer_set_bitmap(mask_layer, mask_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(mask_layer));
  
  #if INVERT
  /*Creates inverted layer, used for inverting the watchface*/
	invert_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(invert_layer));
	#endif
}

static void window_unload(Window *window) 
{
  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_layer);
  
  #if TWELVE_HOUR
  text_layer_destroy(ampm_layer);
  #endif
  
  #if INVERT
  inverter_layer_destroy(invert_layer);
  #endif

  gbitmap_destroy(past_gbitmap);
  gbitmap_destroy(future_gbitmap);
  gbitmap_destroy(mask_gbitmap);
  gbitmap_destroy(present_gbitmap);

  bitmap_layer_destroy(past_bitmap_layer);
  bitmap_layer_destroy(future_bitmap_layer);
  bitmap_layer_destroy(mask_layer);
  bitmap_layer_destroy(present_bitmap_layer);
}

static void tick_handler(struct tm *t, TimeUnits units_changed)
{
  /*-----HOURS-----*/
  //Format
  int last, next, now = t->tm_hour;


  #if TWELVE_HOUR
  //Handle hour-clock wrapping, AM-hours
  if(now >= 0 && now < 12) {
    if(now == 0) {
      now = 12;
      last = 11;
      next = 1;
    } else if(now == 23) {
      last = 22;
      next = 12;
    } else {
      last = now - 1;
      next = now + 1;
      if(last == 0)
        last = 12;
    }
  } else { //Handle hour-clock wrapping, PM-hours
      now -= 12; //Converts to standard time
      if(now == 0)
        now = 12;
    
      if(now == 11) {
        last = 10;
        next = 12;
      } else if(now == 12) {
        last = 11;
        next = 1;
      } else {
          last = now - 1;
          next = now + 1;
      }
      
      //handles 12am and 12pm
      if(last == 0)
        last = 12;
      if(next == 0)
         next = 12;
  }
  #else
  //Handle clock wrapping
  if(now == 0)
  {
    last = 11;
    next = 1;
  } 
  else if(now == 23)
  {
    last = 22;
    next = 0;
  }
  else
  {
    last = now - 1;
    next = now + 1;
  }
  #endif

  if(last >= 10 && now >= 10 && next < 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "%d %d 0%d", last, now, next);
  }
  else if(last >= 10 && now < 10 && next >= 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "%d 0%d %d", last, now, next);
  }
  else if(last >= 10 && now < 10 && next < 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "%d 0%d 0%d", last, now, next);
  }
  else if(last < 10 && now >= 10 && next >= 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "0%d %d %d", last, now, next);
  }
  else if(last < 10 && now >= 10 && next < 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "0%d %d 0%d", last, now, next);
  }
  else if(last < 10 && now < 10 && next >= 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "0%d 0%d %d", last, now, next);
  }
  else if(last < 10 && now < 10 && next < 10)
  {
    snprintf(hour_buff, 16 * sizeof(char), "0%d 0%d 0%d", last, now, next);
  }
  else
  {
    snprintf(hour_buff, 16 * sizeof(char), "%d %d %d", last, now, next);
  }

  //Set hours
  text_layer_set_text(hour_layer, hour_buff);
  #if TWELVE_HOUR
	//Set am/pm
	strftime(ampm_text, sizeof(ampm_text), "%p", t);
  text_layer_set_text(ampm_layer, ampm_text);
  #endif


  /*-----MINUTES-----*/
  //Format minute
  last = 0, next = 0, now = t->tm_min;

  //Handle minute-clock wrapping
  if(now == 0)
  {
    last = 59;
    next = 1;
  } 
  else if(now == 59)
  {
    last = 58;
    next = 0;
  }
  else
  {
    last = now - 1;
    next = now + 1;
  }

  if(last >= 10 && now >= 10 && next < 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "%d %d 0%d", last, now, next);
  }
  else if(last >= 10 && now < 10 && next >= 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "%d 0%d %d", last, now, next);
  }
  else if(last >= 10 && now < 10 && next < 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "%d 0%d 0%d", last, now, next);
  }
  else if(last < 10 && now >= 10 && next >= 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "0%d %d %d", last, now, next);
  }
  else if(last < 10 && now >= 10 && next < 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "0%d %d 0%d", last, now, next);
  }
  else if(last < 10 && now < 10 && next >= 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "0%d 0%d %d", last, now, next);
  }
  else if(last < 10 && now < 10 && next < 10)
  {
    snprintf(minute_buff, 16 * sizeof(char), "0%d 0%d 0%d", last, now, next);
  }
  else
  {
    snprintf(minute_buff, 16 * sizeof(char), "%d %d %d", last, now, next);
  }

  //Set minutes
  text_layer_set_text(minute_layer, minute_buff);
}

static void init(void) 
{
  hour_buff = malloc(16 * sizeof(char));
  minute_buff = malloc(16 * sizeof(char));

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);

  window_stack_push(window, true);
}

static void deinit(void) 
{
  free(hour_buff);
  free(minute_buff);

  window_destroy(window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}
