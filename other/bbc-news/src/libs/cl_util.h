/* Utility functions for my own development */

//V 2.0.1 - 13/06/14

#include <pebble.h>

#ifndef CL_UTIL_H
#define CL_UTIL_H

void cl_set_debug(bool b);
TextLayer* cl_text_layer_create(GRect location, GColor colour, GColor background, bool custom_font, GFont *g_font, const char *res_id, GTextAlignment alignment);
void cl_app_message_open(int inbound_size, int outbound_size, AppMessageInboxReceived in_received_handler);
void cl_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay, AnimationStoppedHandler stopped_handler);
void cl_send_int(int key, int cmd);
void cl_applog(char* message);
void cl_interpret_message_result(AppMessageResult app_message_error);
void cl_breakpoint();
void cl_fill_chamfer_rect(GContext *ctx, int margin, int width, int height);

#endif