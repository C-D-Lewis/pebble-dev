#include "data.h"

static PersistData s_persist_data;

void data_init() {
  // Test - always start fresh
  persist_delete(SK_PersistData);

  // Set defaults
  s_persist_data.day_bg_color = GColorWhite;
  s_persist_data.day_block_color = GColorBlack;
  s_persist_data.day_shadow_color = GColorLightGray;
  s_persist_data.day_void_color = PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite);
  s_persist_data.night_bg_color = GColorBlack;
  s_persist_data.night_block_color = GColorWhite;
  s_persist_data.night_shadow_color = GColorDarkGray;
  s_persist_data.night_void_color = PBL_IF_COLOR_ELSE(GColorLightGray, GColorBlack);

  // Read existing data if it exists
  if (persist_exists(SK_PersistData)) {
    persist_read_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
  }
}

void data_deinit() {
  persist_write_data(SK_PersistData, &s_persist_data, sizeof(PersistData));
}

/***************************** Getters / setters ******************************/

PersistData* data_get_persist_data() {
  return &s_persist_data;
}
