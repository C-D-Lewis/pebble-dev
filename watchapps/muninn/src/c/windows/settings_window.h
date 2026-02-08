#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/comm.h"
#include "../modules/data.h"
#include "../modules/util.h"
#include "../modules/scalable.h"

#include "message_window.h"
#include "stats_window.h"

void settings_window_push();

void settings_window_reload();
