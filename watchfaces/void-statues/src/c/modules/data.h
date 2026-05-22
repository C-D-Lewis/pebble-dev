#pragma once

#include <pebble.h>

typedef enum {
  SK_PersistData = 10,
} StorageKeys;

typedef struct {
  // Color options
  GColor day_bg_color;
  GColor day_shadow_color;
  GColor day_block_color;
  GColor day_void_color;
  GColor night_bg_color;
  GColor night_shadow_color;
  GColor night_block_color;
  GColor night_void_color;
} PersistData;

void data_init();
void data_deinit();

PersistData* data_get_persist_data();
