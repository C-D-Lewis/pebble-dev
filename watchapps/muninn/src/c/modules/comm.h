#pragma once

#include <pebble.h>

#include "data.h"

#include "../windows/settings_window.h"
#include "../windows/stats_window.h"

void comm_init();

void comm_deinit();

void comm_request_deletion();

void comm_request_sync_stats();
