#pragma once

#include <pebble.h>

#include <pebble-scalable/pebble-scalable.h>

#include "../modules/data.h"
#include "../modules/scalable.h"

#include "../libs/InverterLayerCompat.h"

#include "../util/util.h"

void main_window_push();

void main_window_reload_config(bool delay);
