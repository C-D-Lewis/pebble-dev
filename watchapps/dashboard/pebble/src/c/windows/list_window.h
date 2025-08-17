#pragma once

#include "dialog_window.h"
#include "schedule_window.h"
#include "wakeup_window.h"

#include "../config.h"
#include "../types.h"

char* list_window_get_type_string(int type);  // TODO Decouple and move to types.h

void list_window_push();
