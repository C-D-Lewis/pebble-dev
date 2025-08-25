/* Utility functions for my own development */

//V 1.2.0

#include <pebble.h>

#ifndef CL_UTIL_H
#define CL_UTIL_H

void cl_set_debug(bool b);
TextLayer* cl_init_text_layer(GRect location, GColor colour, GColor background, bool custom_font, int custom_res_id, const char *res_id, GTextAlignment alignment);
void cl_init_app_message(int inbound_size, int outbound_size, AppMessageInboxReceived in_received_handler);
void cl_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay);
void cl_send_int(uint8_t key, uint8_t cmd);
void cl_applog(char* message);
void cl_interpret_message_result(AppMessageResult app_message_error);
void cl_breakpoint();
void cl_fill_chamfer_rect(GContext *ctx, int margin, int width, int height);

#endif