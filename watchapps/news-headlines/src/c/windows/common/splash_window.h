#pragma once

#include <pebble.h>

#include "../../modules/data.h"
#include "../../modules/scalable.h"

#if defined(PBL_ROUND)
  #include "../round/stories_window.h"
#else
  #include "../rect/stories_window.h"
#endif

// Push the splash Window
void splash_window_push();

// Set the 'progress' while downloading stories
void splash_window_set_progress(int progress);

// Cancel the loading timeout
void splash_window_cancel_timeout();

// JS is ready, request data
void splash_window_begin();
