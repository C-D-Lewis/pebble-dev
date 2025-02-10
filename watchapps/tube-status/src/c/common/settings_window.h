#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/settings.h"
#include "../modules/comm.h"

#include "splash_window.h"

typedef enum {
  SettingsWindowRowSubscription = 0,
  SettingsWindowRowServerStatus,
  SettingsWindowRowAbout,

  SettingsWindowRowMax
} SettingsWindowRow;

void settings_window_push();

void settings_window_update_server_status(ServerStatus s);