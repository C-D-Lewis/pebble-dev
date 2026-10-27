#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Weather,
  SFI_Date,
  SFI_DateLarge,
} ScalableFontId;

void scalable_init();
