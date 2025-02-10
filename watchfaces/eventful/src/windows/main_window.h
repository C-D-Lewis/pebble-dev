#pragma once

#include <pebble.h>

void main_window_push();

void main_window_set_time(struct tm *tick_time);

void main_window_set_unread_count(int count);

void main_window_set_next_event(char *event_text);
