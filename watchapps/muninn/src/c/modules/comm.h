#pragma once

#include <pebble.h>

#include "data.h"

void comm_init();

void comm_deinit();

void comm_push_timeline_pins();

void comm_get_last_timestamp();

void comm_send_samples(int last_ts);
