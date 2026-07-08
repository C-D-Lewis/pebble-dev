#pragma once

#include <pebble.h>
#include <pebble-packet/pebble-packet.h>
#include "../config.h"
#include "../windows/main_window.h"
#include "data.h"

void comm_init();

void comm_deinit();

void comm_sync_data();

void comm_toggle(ToggleType type);
