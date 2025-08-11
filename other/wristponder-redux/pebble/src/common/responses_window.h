#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/comm.h"
#include "../modules/data.h"

#include "sending_window.h"

void responses_window_push(ContactType contact_type, int contact_index);

void responses_window_reload_data();
