#pragma once

#include <pebble.h>

typedef enum {
  // Watch: Acknowledged, send data. Phone: JS is ready
  AppMessageKeyJSReady = 14,
  // Watch: New subscription state
  AppMessageKeySubscriptionState,
  // Watch: Get me that server status. Phone: Here's is the status
  AppMessageKeyServerStatus,
  // Watch: N/A. Phone: Here's how far sending has progressed
  AppMessageKeyIndex
} AppMessageKey;

#include "../config.h"
#include "data.h"

#include "../rect/line_window.h"
#include "../round/line_window.h"
#include "../common/splash_window.h"
#include "../common/settings_window.h"

#include <pebble-packet/pebble-packet.h>

#define COMM_INBOX_SIZE 2026
#define COMM_OUTBOX_SIZE 656

void comm_init();
void comm_deinit();

void comm_get_server_status();

void comm_request_data();

void comm_update_subscription_state();
