#include "cl_util.h"

static bool DEBUG = false;
static bool APP_MESSAGE_OPEN = false;

/*
 * Set the debug value for these functions
 */
void cl_set_debug(bool b)
{
	DEBUG = b;
}

/*
 * Set up a TextLayer in one line. _add_child() left out for flexibility
 */
TextLayer* cl_text_layer_create(GRect location, GColor colour, GColor background, bool custom_font, GFont *g_font, const char *res_id, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  if(custom_font == true)
  {
    text_layer_set_font(layer, g_font);
  }
  else
  {
    text_layer_set_font(layer, fonts_get_system_font(res_id));
  }
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}

/*
 * Internal _in_dropped() handler because it rarely does anything else
 */
static void in_dropped_handler(AppMessageResult reason, void *context) 
{ 
	if(DEBUG == true)
	{
		cl_interpret_message_result(reason);
	}
}

/*
 * Out failed handler
 */
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) 
{
  if(DEBUG == true)
  {
    cl_interpret_message_result(reason);
  }
}

/*
 * Initialise basic AppMessage functionality
 */
void cl_app_message_open(int inbound_size, int outbound_size, AppMessageInboxReceived in_received_handler)
{
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  if(APP_MESSAGE_OPEN == false)
  {
    app_message_open(inbound_size, outbound_size);
    APP_MESSAGE_OPEN = true;

    if(DEBUG)
    {
      cl_applog("AppMessage opened.");
    }
  }
}

void cl_deinit_app_message()
{
  app_message_deregister_callbacks();

  APP_MESSAGE_OPEN = false;

  if(DEBUG)
  {
    cl_applog("AppMessage closed.");
  }
}

/*
 * Internal Animation disposal
 */
static void on_animation_stopped(Animation *anim, bool finished, void *context)
{
  property_animation_destroy((PropertyAnimation*) anim);
}

/*
 * Animate a layer with duration and delay
 */
void cl_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay, AnimationStoppedHandler stopped_handler)
{
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);
  
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler)on_animation_stopped
  };

  if(stopped_handler != NULL)
  {
    handlers.stopped = (AnimationStoppedHandler)stopped_handler;
  }
  
  animation_set_handlers((Animation*) anim, handlers, NULL);
  
  animation_schedule((Animation*) anim);
}

/*
 * Send a simple integer key-value pair over AppMessage
 */
void cl_send_int(int key, int cmd)
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet value = TupletInteger(key, cmd);
  dict_write_tuplet(iter, &value);
  
  app_message_outbox_send();
}

/*
 * Convenience app_log shortcut
 */
void cl_applog(char* message)
{
  app_log(APP_LOG_LEVEL_INFO, "C", 0, message);
}

/*
 * Convert AppMessageResult to readable console output. Pebble SDK really needs this!
 */
void cl_interpret_message_result(AppMessageResult app_message_error)
{
  if(app_message_error == APP_MSG_OK)
  {
    cl_applog("APP_MSG_OK");
  } 

  else if(app_message_error == APP_MSG_SEND_TIMEOUT)
  {
    cl_applog("APP_MSG_SEND_TIMEOUT");
  } 

  else if(app_message_error == APP_MSG_SEND_REJECTED)
  {
    cl_applog("APP_MSG_SEND_REJECTED");
  }

  else if(app_message_error == APP_MSG_NOT_CONNECTED)
  {
    cl_applog("APP_MSG_NOT_CONNECTED");
  }

  else if(app_message_error == APP_MSG_APP_NOT_RUNNING)
  {
    cl_applog("APP_MSG_APP_NOT_RUNNING");
  }

  else if(app_message_error == APP_MSG_INVALID_ARGS)
  {
    cl_applog("APP_MSG_INVALID_ARGS");
  }

  else if(app_message_error == APP_MSG_BUSY)
  {
    cl_applog("APP_MSG_BUSY");
  }

  else if(app_message_error == APP_MSG_BUFFER_OVERFLOW)
  {
    cl_applog("APP_MSG_BUFFER_OVERFLOW");
  }

  else if(app_message_error == APP_MSG_ALREADY_RELEASED)
  {
    cl_applog("APP_MSG_ALREADY_RELEASED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_ALREADY_REGISTERED)
  {
    cl_applog("APP_MSG_CALLBACK_ALREADY_REGISTERED");
  }

  else if(app_message_error == APP_MSG_CALLBACK_NOT_REGISTERED)
  {
    cl_applog("APP_MSG_CALLBACK_NOT_REGISTERED");
  }

  else if(app_message_error == APP_MSG_OUT_OF_MEMORY)
  {
    cl_applog("APP_MSG_OUT_OF_MEMORY");
  }
}

/*
 * Quick breakpoint for function progress debugging
 */
void cl_breakpoint()
{
  cl_applog("***BREAKPOINT***");
}

/**
  * Fill a chamfered rectangle
  */
void cl_fill_chamfer_rect(GContext *ctx, int margin, int width, int height)
{
  //Range check
  if(width < 2 * margin)
  {
    width = 2 * margin;
  }

  //Draw
  for(int y = 0; y < margin; y++)
  {
    graphics_draw_line(ctx, GPoint(margin - y - 1, y), GPoint(width - (margin - y), y));
  }
  for(int y = margin; y < height - margin; y++)
  {
    graphics_draw_line(ctx, GPoint(0, y), GPoint(width, y));
  }
  for(int y = 0; y < margin; y++)
  {
    graphics_draw_line(ctx, GPoint(y, height - margin + y), GPoint(width - y - 1, height - margin + y));
  }
}

/**
 * Get a tm object
 */
struct tm* cl_get_time()
{
  time_t temp = time(NULL); 
  return localtime(&temp);
}