#pragma once

#include <pebble.h>

#include "../config.h"
#include "../modules/data.h"
#include "../modules/settings.h"
#include "settings_window.h"
#include "detail_window.h"

// Layout values
#define STORIES_WINDOW_CHROME_HEIGHT 16 + 81  // with image height
#define STORIES_WINDOW_TITLE_HEIGHT  2 * STORIES_WINDOW_CHROME_HEIGHT + 2
#define STORIES_WINDOW_PADDING       10
#define STORIES_WINDOW_ANIM_OFFSET   20

// Push the stories Window
void stories_window_push();
