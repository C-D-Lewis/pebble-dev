#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Dial,
  SFI_Date,
  SFI_Time,
} ScalableFontId;

void scalable_init();
