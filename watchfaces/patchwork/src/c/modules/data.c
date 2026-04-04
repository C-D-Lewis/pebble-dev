#include "data.h"

static char s_palette[32];

static void save_all() {
  persist_write_string(MESSAGE_KEY_PALETTE, s_palette);
}

void data_init() {
  if (persist_exists(MESSAGE_KEY_PALETTE)) {
    persist_read_string(MESSAGE_KEY_PALETTE, s_palette, sizeof(s_palette));
  } else {
    snprintf(s_palette, sizeof(s_palette), "pastel");
  }
}

void data_deinit() {
  save_all();
}

char* data_get_palette() {
  return s_palette;
}

void data_set_palette(char* palette) {
  snprintf(s_palette, sizeof(s_palette), "%s", palette);
}
