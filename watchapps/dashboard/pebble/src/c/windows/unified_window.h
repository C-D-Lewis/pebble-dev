#pragma once

#include <pebble.h>

#include "dialog_window.h"
#include "list_window.h"
#include "wakeup_window.h"

#include "../types.h"

#include "../modules/comm.h"
#include "../modules/icons.h"

#include "../util/animation.h"

void unified_window_push();

void unified_window_remove_from_stack();

void unified_window_reload();

void unified_window_jump_to(int type);

void unified_window_set_interaction_enabled(bool b);
