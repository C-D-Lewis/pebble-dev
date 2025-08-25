#include <pebble.h>
#include "cl_util.h"

#define CHROME_HEIGHT 16

#define KEY_REQUEST_TEMPERATURE 0

static Window *window;
static BitmapLayer *past_bitmap_layer, *future_bitmap_layer, *mask_layer, *present_bitmap_layer;
static GBitmap *past_gbitmap, *future_gbitmap, *mask_gbitmap, *present_gbitmap;
static TextLayer *hour_layer, *minute_layer, *date_layer, *temp_layer;
static char *hour_buff, *minute_buff, *date_buff, *temp_buff;

static void tick_handler(struct tm *t, TimeUnits units_changed)
{
  //Format
  int last, next, now = t->tm_hour;

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

  //Set
  text_layer_set_text(hour_layer, hour_buff);

  //Format minute
  last = 0, next = 0, now = t->tm_min;

  //Handle clock wrapping
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

  //Get date
  strftime(date_buff, sizeof("22 12"), "%e %m", t);
  text_layer_set_text(date_layer, date_buff);

  //Sometimes request weather
  if(t->tm_min % 15 == 0)
  {
    cl_send_int(KEY_REQUEST_TEMPERATURE, 0);
  }
}

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

  date_layer = cl_init_text_layer(GRect(5, 165 - CHROME_HEIGHT, 139, 30), GColorBlack, GColorClear, true, RESOURCE_ID_FONT_MEGAFONT_15, 0, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  temp_layer = cl_init_text_layer(GRect(85, 165 - CHROME_HEIGHT, 139, 30), GColorBlack, GColorClear, true, RESOURCE_ID_FONT_MEGAFONT_15, 0, GTextAlignmentLeft);
  text_layer_set_text(temp_layer, "...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temp_layer));

  mask_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_MASK);
  mask_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(mask_layer, GCompOpOr);
  bitmap_layer_set_bitmap(mask_layer, mask_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(mask_layer));

  //Init weather
  cl_send_int(KEY_REQUEST_TEMPERATURE, 0);

  //Init time
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  tick_handler(t, MINUTE_UNIT);
}

static void window_unload(Window *window) 
{
  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(temp_layer);

  gbitmap_destroy(past_gbitmap);
  gbitmap_destroy(future_gbitmap);
  gbitmap_destroy(mask_gbitmap);
  gbitmap_destroy(present_gbitmap);

  bitmap_layer_destroy(past_bitmap_layer);
  bitmap_layer_destroy(future_bitmap_layer);
  bitmap_layer_destroy(mask_layer);
  bitmap_layer_destroy(present_bitmap_layer);
}

static void process_tuple(Tuple *t)
{
  switch(t->key) {
  case KEY_REQUEST_TEMPERATURE:
    snprintf(temp_buff, 8 * sizeof(char), "%s C", t->value->cstring);
    text_layer_set_text(temp_layer, temp_buff);
    break;
  default:
    break;
  }
}

static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  Tuple *t = dict_read_first(iterator);
  while(t)
  {
    process_tuple(t);
    t = dict_read_next(iterator);
  }
}

static void init(void) 
{
  hour_buff = malloc(16 * sizeof(char));
  minute_buff = malloc(16 * sizeof(char));
  date_buff = malloc(8 * sizeof(char));
  temp_buff = malloc(8 * sizeof(char));

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  cl_init_app_message(
    app_message_inbox_size_maximum(), 
    app_message_outbox_size_maximum(),
    (AppMessageInboxReceived)in_recv_handler
  );

  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);

  window_stack_push(window, true);
}

static void deinit(void) 
{
  free(hour_buff);
  free(minute_buff);
  free(date_buff);
  free(temp_buff);

  window_destroy(window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}
