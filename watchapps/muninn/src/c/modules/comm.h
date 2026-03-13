#pragma once

#include <pebble.h>

#include "../config.h"

#include "data.h"

#include "../windows/settings_window.h"
#include "../windows/message_window.h"

void comm_init();

void comm_deinit();

#ifdef FEATURE_SYNC
void comm_request_deletion();

void comm_request_sync_stats();

void comm_upload_history();
#endif
