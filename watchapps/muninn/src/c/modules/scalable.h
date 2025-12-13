#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Small = 0,
  SFI_Medium,
  SFI_MediumBold,
  SFI_Large,
  SFI_LargeBold,
} ScalableFontId;

void scalable_init();
