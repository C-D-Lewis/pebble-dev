#pragma once

#include <pebble.h>

#include "../config.h"

typedef struct {
  int days_remaining;
  int battery_life;
  int last_week_rate;
} AppState;

void data_init();
void data_deinit();

AppState* data_get_app_state();