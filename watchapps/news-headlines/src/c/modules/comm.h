#pragma once

#include <pebble.h>

#include "../config.h"
#include "data.h"
#include "settings.h"
#include "../types.h"
#if defined(PBL_ROUND)
  #include "../windows/round/stories_window.h"
#else
  #include "../windows/rect/stories_window.h"
#endif
#include "../windows/common/splash_window.h"
#include "../windows/common/settings_window.h"

#define COMM_TIMEOUT_MS 10000

// Initialize app communication
void comm_init();

// Send web settings to JS
void comm_send_settings();

// Faster AppMessage
void comm_set_fast(bool fast);
