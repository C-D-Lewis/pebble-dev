#include "data.h"

static int s_update_freq_mins;

void data_init() {
  if(persist_exists(MESSAGE_KEY_UpdateFrequency)) {
    s_update_freq_mins = persist_read_int(MESSAGE_KEY_UpdateFrequency);
  } else {
    s_update_freq_mins = 0;
  }
}

void data_set_update_frequency(int mins) {
  s_update_freq_mins = mins;
  persist_write_int(MESSAGE_KEY_UpdateFrequency, mins);
}

int data_get_update_frequency() {
  return s_update_freq_mins;
}
