#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/util.h"
#include "../modules/bitmaps.h"
#include "../modules/scalable.h"

void message_window_push(char *text, bool do_vibe, bool do_dismiss);
