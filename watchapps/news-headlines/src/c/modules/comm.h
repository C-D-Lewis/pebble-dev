#pragma once

#include <pebble.h>

#include "../config.h"
#include "data.h"
#include "settings.h"
#include "../types.h"
#include "../windows/stories_window.h"
#include "../windows/splash_window.h"
#include "../windows/settings_window.h"

#define COMM_TIMEOUT_MS 10000

// Initialize app communication
void comm_init();

// Send web settings to JS
void comm_send_settings();

// Faster AppMessage
void comm_set_fast(bool fast);
