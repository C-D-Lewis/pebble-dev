#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_MediumBold = 0,
  SFI_Large,
} ScalableFontId;

void scalable_init();
