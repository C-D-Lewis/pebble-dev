#pragma once

#include <pebble.h>
#include <pebble-isometric/pebble-isometric.h>

#include "../config.h"
#include "drawing.h"

void drawing_draw_number(int number, GPoint pos);

void drawing_set_colors(GColor box_color, GColor shadow_color, GColor clarity_color);

void drawing_set_is_connected(bool is_connected);
