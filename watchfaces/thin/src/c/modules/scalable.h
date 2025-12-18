#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_SmallBold = 0,
  SFI_MediumBold,
} ScalableFontId;

void scalable_init();
