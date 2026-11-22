#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Small,
  SFI_Medium,
  SFI_Large,
} ScalableFontId;

void scalable_init();
