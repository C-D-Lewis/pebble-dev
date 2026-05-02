#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Label = 0,
  SFI_Date,
  SFI_Time
} ScalableFontId;

void scalable_init();
