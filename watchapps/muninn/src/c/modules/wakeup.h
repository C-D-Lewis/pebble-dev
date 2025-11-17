#pragma once

#include <pebble.h>

#include "../modules/data.h"

void wakeup_schedule_next();

void wakeup_unschedule();

void wakeup_handler();
