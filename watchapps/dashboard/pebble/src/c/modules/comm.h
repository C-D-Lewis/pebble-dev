#pragma once

#include <pebble.h>

#include "../types.h"
#include "../config.h"

#include "../util/test.h"

#include "../windows/dialog_window.h"
#include "../windows/splash_window.h"
#include "../windows/unified_window.h"

void comm_init(uint32_t inbox, uint32_t outbox);

void comm_request_all();

void comm_request_toggle(int type, int state);

bool comm_check_bt(char *failed_notice);
