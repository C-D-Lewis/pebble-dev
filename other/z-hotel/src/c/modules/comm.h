#pragma once

#include <pebble.h>

#include <pebble-packet/pebble-packet.h>
#include <pebble-events/pebble-events.h>

#include "../windows/splash_window.h"
#include "../windows/list_window.h"
#include "data.h"

#define COMM_INBOX_SIZE 1024
#define COMM_OUTBOX_SIZE 128

void comm_init();

void comm_deinit();
