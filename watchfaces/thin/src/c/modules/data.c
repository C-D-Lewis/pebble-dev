#include "data.h"

#define PERSIST_SET 3489

// Cache for speed
static bool s_arr[DataKeyCount];

void data_init() {
  if(!persist_exists(PERSIST_SET)) {
    // Set defaults
    s_arr[DataKeyDate] = true;
    s_arr[DataKeyDay] = true;
    s_arr[DataKeyBT] = true;
    s_arr[DataKeyBattery] = true;
    s_arr[DataKeySecondHand] = true;
    s_arr[DataKeySteps] = true;
  } else {
    // Read previously stored values
    for(int i = 0; i < DataKeyCount; i++) {
      s_arr[i] = persist_read_bool(i);
    }
  }
}

void data_deinit() {
  // Save current values
  persist_write_bool(PERSIST_SET, true);

  for(int i = 0; i < DataKeyCount; i++) {
    persist_write_bool(i, s_arr[i]);
  }
}

void data_set(int key, bool value) {
  s_arr[key] = value;
}

bool data_get(int key) {
  return s_arr[key];
}
