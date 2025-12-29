#pragma once

#include <pebble.h>
#include <pebble-universal-fb/pebble-universal-fb.h>

#include "../config.h"

#include "../modules/data.h"
#include "../modules/scalable.h"


void main_window_push();

void main_window_reload_config(bool delay);
