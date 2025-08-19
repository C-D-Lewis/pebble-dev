#pragma once

#include <pebble.h>
#include <pebble-isometric/pebble-isometric.h>

#include "drawing.h"
#include "../config.h"

void drawing_draw_number(int number, GPoint pos);

void drawing_set_colors(GColor box_color, GColor shadow_color);
