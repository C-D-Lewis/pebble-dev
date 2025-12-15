#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Small = 0,
  SFI_Medium,
  SFI_MediumBold,
} ScalableFontId;

void scalable_init();
