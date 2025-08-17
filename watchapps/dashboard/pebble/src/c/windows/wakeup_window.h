#pragma once

#include "schedule_window.h"

#include "../types.h"
#include "../modules/comm.h"

char* wakeup_window_get_state_string(int state);  // TODO decouple and move to types.h

void wakeup_window_push(int type, int state);
