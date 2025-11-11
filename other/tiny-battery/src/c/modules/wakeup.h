#pragma once

#include <pebble.h>

#include "../modules/data.h"

#include "../windows/wakeup_window.h"

// No wakeup scheduled anymore
#define WAKEUP_NO_WAKEUP -1

void wakeup_schedule_next();

void wakeup_unschedule();

void wakeup_handler();
