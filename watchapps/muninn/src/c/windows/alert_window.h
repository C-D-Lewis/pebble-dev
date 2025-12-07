#pragma once

#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#include "../modules/data.h"
#include "../modules/util.h"

void alert_window_push(uint32_t res_id, char *message, bool do_vibe, bool do_dismiss);
