#pragma once

#include <pebble.h>

void util_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay);

TextLayer* util_init_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment);
