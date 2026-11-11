#pragma once

#include <pebble.h>
#include <pebble-scalable/pebble-scalable.h>

typedef enum {
  SFI_Medium,
  SFI_MediumBold,

  SFI_StatusLayer,
} ScalableFontId;

void scalable_init();
