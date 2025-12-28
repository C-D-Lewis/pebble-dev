#pragma once

#include <pebble.h>

GBitmap* bitmaps_get(uint32_t res_id);

void bitmaps_destroy(GBitmap *ptr);

void bitmaps_destroy_all();
