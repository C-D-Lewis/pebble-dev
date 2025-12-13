#pragma once

#include <pebble.h>

#include "../modules/data.h"
#include "../modules/util.h"
#include "../modules/scalable.h"

void alert_window_push(uint32_t res_id, char *message, bool do_vibe, bool do_dismiss);
