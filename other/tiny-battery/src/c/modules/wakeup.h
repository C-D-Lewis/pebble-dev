#pragma once

#include <pebble.h>

#include "../modules/data.h"

#include "../windows/wakeup_window.h"

void wakeup_schedule_next();

void wakeup_unschedule();

void wakeup_handler();
