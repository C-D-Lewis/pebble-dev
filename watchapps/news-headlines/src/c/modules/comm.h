#pragma once

#include <pebble.h>

#include "../config.h"
#include "data.h"
#include "settings.h"
#include "../types.h"
#include "../windows/stories_window.h"
#include "../windows/splash_window.h"
#include "../windows/settings_window.h"
#include "../windows/image_window.h"

#define COMM_TIMEOUT_MS 10000

// Initialize app communication
void comm_init();

// Set app comm mode and request data
void comm_request_quantity();

// Send web settings to JS
void comm_send_settings();

// Query the pin server
void comm_get_pin_server_status();

// Faster AppMessage
void comm_set_fast(bool fast);

// Fetch an image
void comm_get_thumbnail(int index);
