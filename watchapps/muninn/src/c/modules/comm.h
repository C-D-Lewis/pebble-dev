#pragma once

#include <pebble.h>

#if !defined(PBL_PLATFORM_APLITE)
#include <pebble-packet/pebble-packet.h>
#endif

#include "data.h"

void comm_init();

void comm_deinit();

void comm_push_timeline_pins();
