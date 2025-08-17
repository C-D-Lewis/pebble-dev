#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/data.h"

#include "../lib/universal_fb/universal_fb.h"

void main_window_push();

void main_window_reload_config(bool delay);
