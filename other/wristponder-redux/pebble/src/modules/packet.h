#pragma once

#include <pebble.h>

#include "../types.h"

bool packet_begin();

void packet_set_type(int type);

void packet_add_int(int key, int value);

void packet_add_string(int key, char *string);

bool packet_send();
