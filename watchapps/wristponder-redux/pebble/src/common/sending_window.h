#pragma once

#include <pebble.h>

#include "../config.h"
#include "../types.h"

#include "../modules/data.h"

void sending_window_push(ContactType type, int contact_index, int response_index, char *transcription);

void sending_window_exit_after_result(SendingResult result);
