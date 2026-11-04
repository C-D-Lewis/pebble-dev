#if defined(PBL_ROUND)
#pragma once

#include <pebble.h>

#include "../../config.h"
#include "../../modules/data.h"

#include "../common/reason_window.h"

#define LINE_WINDOW_RADIUS scl_x(60)
#define LINE_WINDOW_MARGIN scl_x(50)
#define LINE_WINDOW_STROKE_WIDTH scl_x(15)

void line_window_push();

#endif
