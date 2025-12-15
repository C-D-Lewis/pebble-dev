#pragma once

#include <pebble.h>

#include "../../config.h"

#include "../../modules/data.h"
#include "../../modules/settings.h"
#include "../../modules/scalable.h"

#include "../common/settings_window.h"
#include "../common/detail_window.h"

// Layout values
#define STORIES_WINDOW_ANIM_OFFSET 20

// Push the stories Window
void stories_window_round_push();
