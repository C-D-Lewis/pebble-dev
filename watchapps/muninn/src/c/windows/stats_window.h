#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/util.h"
#include "../modules/bitmaps.h"
#include "../modules/scalable.h"
#include "../modules/comm.h"

#ifdef FEATURE_SYNC
void stats_window_push();

void stats_window_reload();
#endif