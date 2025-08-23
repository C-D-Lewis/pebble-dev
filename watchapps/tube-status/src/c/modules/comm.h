#pragma once

#include <pebble.h>

#include <pebble-packet/pebble-packet.h>

#include "../common/splash_window.h"
#include "../config.h"
#include "../rect/line_window.h"
#include "../round/line_window.h"
#include "data.h"

#define COMM_INBOX_SIZE 2048
#define COMM_OUTBOX_SIZE 256

void comm_init();

void comm_deinit();
