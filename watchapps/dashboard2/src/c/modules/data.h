#pragma once

#include <pebble.h>

#include "../config.h"

#include "../windows/main_window.h"

void data_init();
void data_deinit();

AppState* data_get_app_state();

bool data_is_toggle_active(int index);

char data_get_toggle_state(int index);

#ifdef USE_TEST_DATA
void data_test_data_handler();
#endif
