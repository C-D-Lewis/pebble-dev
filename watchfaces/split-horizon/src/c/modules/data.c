#include "data.h"

static bool s_show_date, s_play_animations;

void data_init() {
  if (persist_exists(MESSAGE_KEY_ShowDate)) {
    // Load existing data
    s_show_date = persist_read_bool(MESSAGE_KEY_ShowDate);
    s_play_animations = persist_read_bool(MESSAGE_KEY_PlayAnimations);
  } else {
    // Save defaults
    s_show_date = true;
    s_play_animations = true;
    persist_write_bool(MESSAGE_KEY_ShowDate, s_show_date);
    persist_write_bool(MESSAGE_KEY_PlayAnimations, s_play_animations);
  }
}

void data_deinit() {
}

bool data_get_boolean(uint32_t key) {
  // Can't use switch with MESSAGE_KEY_ constants directly
  if (key == MESSAGE_KEY_ShowDate) {
    return s_show_date;
  } else if (key == MESSAGE_KEY_PlayAnimations) {
    return s_play_animations;
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key for data_get_boolean: %lu", (unsigned long)key);
    return false; // Unknown key
  }
}

void data_set_boolean(uint32_t key, bool value) {
  if (key == MESSAGE_KEY_ShowDate) {
    s_show_date = value;
    persist_write_bool(MESSAGE_KEY_ShowDate, value);
  } else if (key == MESSAGE_KEY_PlayAnimations) {
    s_play_animations = value;
    persist_write_bool(MESSAGE_KEY_PlayAnimations, value);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown key for data_set_boolean: %lu", (unsigned long)key);
    // Handle unknown key if necessary
  }
}
