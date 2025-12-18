#pragma once

#include <pebble.h>

#include "data.h"

void health_init();

bool health_is_health_available();

int health_get_step_count();

bool health_step_data_is_available();
