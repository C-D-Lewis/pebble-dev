#pragma once

#include <pebble.h>

#include "../config.h"
#include "../types.h"
#include "../modules/comm.h"
#include "splash_window.h"
#include "../modules/settings.h"

#define SETTINGS_WINDOW_NUM_SETTINGS 8

// Push the settings Window
void settings_window_push();

// Update the server status menu row
void settings_window_update_pin_server_status(ServerStatus s);
