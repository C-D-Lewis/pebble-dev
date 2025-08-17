#pragma once

#include <pebble.h>

#include "data.h"

void icons_load();

void icons_unload();

GBitmap* icons_get_with_type(int type);
