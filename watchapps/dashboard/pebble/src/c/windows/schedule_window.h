#pragma once

#include <pebble.h>

#include "dialog_window.h"
#include "list_window.h"

#include "../config.h"
#include "../types.h"

#include "../modules/data.h"

#include "../lib/new_number_window/new_number_window.h"

WeekDay schedule_window_get_tomorrow_weekday();  // TODO move to types.h

void schedule_window_push(int position);
