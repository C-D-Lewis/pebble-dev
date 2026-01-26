#pragma once

#include <pebble.h>

#include "data.h"

#include "../windows/settings_window.h"

void comm_init();

void comm_deinit();

void comm_request_deletion();
