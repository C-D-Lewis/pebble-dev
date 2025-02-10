#pragma once

#include <pebble.h>

typedef enum {
  DataKeySubscription = 0,
} DataKey;

#include "../config.h"
#include "settings.h"

void data_init();
void data_deinit();

char* data_get_line_name(int type);

char* data_get_line_state(int type);

GColor data_get_line_color(int type);

GColor data_get_line_state_color(int type);
