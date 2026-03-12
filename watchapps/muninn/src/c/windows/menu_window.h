#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/comm.h"
#include "../modules/data.h"
#include "../modules/util.h"
#include "../modules/scalable.h"

#include "message_window.h"
#include "stats_window.h"
#include "settings_window.h"

void menu_window_push();

void menu_window_reload();

#ifdef FEATURE_SYNC
void menu_window_set_upload_status(const char *status);
#endif
