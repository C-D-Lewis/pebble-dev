#pragma once

#include <pebble.h>

#include "../config.h"
#include "../types.h"
#include "../modules/comm.h"
#include "splash_window.h"
#include "../modules/settings.h"

#define SETTINGS_WINDOW_NUM_SETTINGS 5

// Push the settings Window
void settings_window_push();
