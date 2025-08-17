#pragma once

#include <pebble.h>

#include "data.h"

void health_init();

bool is_health_available();

int get_step_count();

bool step_data_is_available();