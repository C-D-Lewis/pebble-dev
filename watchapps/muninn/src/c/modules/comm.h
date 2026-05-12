#pragma once

#include <pebble.h>

#include "../config.h"

#include "data.h"

#include "../windows/message_window.h"
#include "../windows/menu_window.h"

void comm_init();

void comm_deinit();

#ifdef FEATURE_SYNC
void comm_upload_history();
#endif
