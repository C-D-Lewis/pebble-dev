#pragma once

#include <pebble.h>

#include <pebble-packet/pebble-packet.h>
#include <pebble-events/pebble-events.h>

#include "../config.h"
#include "../windows/common/splash_window.h"
#if defined(APP_VARIANT_TRANSIT_STATUS)
#include "../windows/transit_status/transit_system_selection_window.h"
#endif
#include "../windows/rect/line_window.h"
#include "../windows/round/line_window.h"
#include "data.h"

#define COMM_INBOX_SIZE 1024
#define COMM_OUTBOX_SIZE 128

void comm_init();

void comm_deinit();

void comm_request_transit_system(int index);