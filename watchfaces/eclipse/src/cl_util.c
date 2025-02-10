#include "cl_util.h"

static bool CL_DEBUG = false;
static bool CL_APP_MESSAGE_OPEN = false;

/*
 * Set the debug value for these functions
 */
void cl_set_debug(bool b)
{
	CL_DEBUG = b;
}

/*
 * Set up a TextLayer in one line. _add_child() left out for flexibility
 */
TextLayer* cl_init_text_layer(GRect location, GColor colour, GColor background, bool custom_font, int custom_res_id, const char *res_id, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  if(custom_font == true)
  {
    ResHandle handle = resource_get_handle(custom_res_id);
    text_layer_set_font(layer, fonts_load_custom_font(handle));
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
static void cl_in_dropped_handler(AppMessageResult reason, void *context) 
{ 
	if(CL_DEBUG == true)
	{
		cl_interpret_message_result(reason);
  }
}

/*
 * Initialise basic AppMessage functionality
 */
void cl_init_app_message(int inbound_size, int outbound_size, AppMessageInboxReceived in_received_handler)
{
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(cl_in_dropped_handler);
  if(CL_APP_MESSAGE_OPEN == false)
  {
    app_message_open(inbound_size, outbound_size);
    CL_APP_MESSAGE_OPEN = true;
  }
}

/*
 * Internal Animation disposal
 */
static void cl_on_animation_stopped(Animation *anim, bool finished, void *context)
{
  property_animation_destroy((PropertyAnimation*) anim);
}

/*
 * Animate a layer with duration and delay
 */
void cl_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay)
{
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);
  
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler) cl_on_animation_stopped
  };
  animation_set_handlers((Animation*) anim, handlers, NULL);
  
  animation_schedule((Animation*) anim);
}

/*
 * Send a simple integer key-value pair over AppMessage
 */
void cl_send_int(uint8_t key, uint8_t cmd)
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
  app_log(APP_LOG_LEVEL_INFO, "wristponder.c", 0, message);
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