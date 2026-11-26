#pragma once

#include <pebble.h>

#include <pebble-packet/pebble-packet.h>

#include "../config.h"
#include "data.h"

#include "../windows/common/splash_window.h"
#include "../windows/rect/line_window.h"
#include "../windows/round/line_window.h"

void comm_init();

void comm_deinit();

int comm_get_lines_received();
