/*********************************************************************/
/* Simple alert mechanism for Pebble apps                            */
/* Author: Chris Lewis                                               */
/* Modified: 21/02/14                                                */
/* Freely available to use with link to https://github.com/C-D-Lewis */
/*********************************************************************/

#include <pebble.h>

#ifndef ALERT_H
#define ALERT_H

//Function prototypes - see alert.c for details
void alert_show(Window *window, const char *title, const char *body, const int duration);
void alert_show_sticky(Window *window, const char *title, const char *body);
void alert_update(const char *new_title, const char *new_body, const int new_duration);
void alert_cancel();
bool alert_is_visible();

#endif