#pragma once

#include <pebble.h>

#include "../modules/data.h"
#include "stories_window.h"

// Push the splash Window
void splash_window_push();

// Set the 'progress' while downloading stories
void splash_window_set_progress(int progress);

// Cancel the loading timeout
void splash_window_cancel_timeout();

// JS is ready, request data
void splash_window_begin();
