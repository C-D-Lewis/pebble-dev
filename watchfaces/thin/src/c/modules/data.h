#pragma once

#include <pebble.h>

typedef enum {
  DataKeyDate = 0,
  DataKeyDay,
  DataKeyBT,
  DataKeyBattery,
  DataKeySecondHand,
  DataKeySteps,

  DataKeyCount
} DataKey;

void data_init();

void data_deinit();

void data_set(int key, bool value);

bool data_get(int key);
