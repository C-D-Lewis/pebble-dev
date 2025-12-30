#pragma once

#include <pebble.h>

#include <pebble-packet/pebble-packet.h>

#include "data.h"

void comm_init();

void comm_deinit();

void comm_push_timeline_pins();
