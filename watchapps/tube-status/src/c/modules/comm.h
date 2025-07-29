#pragma once

#include <pebble.h>

typedef enum {
  // Watch: Acknowledged, send data. Phone: JS is ready
  AppMessageKeyJSReady = 11,
} AppMessageKey;

#include "../config.h"
#include "data.h"

#include "../rect/line_window.h"
#include "../round/line_window.h"
#include "../common/splash_window.h"

#include <pebble-packet/pebble-packet.h>

#define COMM_INBOX_SIZE 4096
#define COMM_OUTBOX_SIZE 1024

void comm_init();
void comm_deinit();

void comm_request_data();
