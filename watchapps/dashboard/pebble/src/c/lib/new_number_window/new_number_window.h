#pragma once

#include <pebble.h>

typedef void(NewNumberWindowCallback)(int);

void new_number_window_push(char *title, int min, int max, int step, NewNumberWindowCallback callback);
