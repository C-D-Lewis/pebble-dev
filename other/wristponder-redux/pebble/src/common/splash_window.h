#pragma once

#include <pebble.h>

#include "../config.h"
#include "../modules/comm.h"
#include "../modules/data.h"

#include "contact_window.h"

#include "../lib/version_check/version_check.h"

#define SPLASH_WINDOW_TIMEOUT_DELAY 1500

void splash_window_push();

void splash_window_pop();
