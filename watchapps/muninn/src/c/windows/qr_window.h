#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/util.h"
#include "../modules/bitmaps.h"
#include "../modules/data.h"
#include "../modules/scalable.h"

#ifdef FEATURE_SYNC
void qr_window_push();
#endif
