#pragma once

#include <pebble.h>

#include "data.h"

#include "../main.h"

typedef enum {
  AppKeyColorBackground = 0,
  AppKeyColorSides,
  AppKeyColorFace,
  AppKeyAnimations,
  AppKeyBluetooth
} AppKey;

void comm_init(int inbox, int outbox);
