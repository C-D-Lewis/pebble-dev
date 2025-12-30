#pragma once

#include <pebble.h>

#include "../modules/data.h"

#include "../windows/alert_window.h"

void wakeup_schedule_next();

void wakeup_unschedule();

void wakeup_handler();

bool wakeup_handle_missed();
