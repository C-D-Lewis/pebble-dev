#include "bitmaps.h"

#define MAX_BITMAPS 32

GBitmap *s_arr[MAX_BITMAPS];
uint32_t s_res_ids[MAX_BITMAPS];

GBitmap* bitmaps_get(uint32_t res_id) {
  // APP_LOG(APP_LOG_LEVEL_INFO, "Loading bitmap %d (heap %d)", (int)res_id, heap_bytes_free());

  // Use cache if already loaded
  for (int i = 0; i < MAX_BITMAPS; i++) {
    if (s_res_ids[i] == res_id && s_arr[i] != NULL) {
      // APP_LOG(APP_LOG_LEVEL_INFO, "r");
      return s_arr[i];
    }
  }

  // Find next free slot
  for (int i = 0; i < MAX_BITMAPS; i++) {
    if (s_arr[i] == NULL) {
      // APP_LOG(APP_LOG_LEVEL_INFO, "a");
      s_arr[i] = gbitmap_create_with_resource(res_id);
      s_res_ids[i] = res_id;
      return s_arr[i];
    }
  }

  // APP_LOG(APP_LOG_LEVEL_ERROR, "Cannot load bitmap %d", (int)res_id);
  return NULL;
}

void bitmaps_destroy_ptr(GBitmap *ptr) {
  for (int i = 0; i < MAX_BITMAPS; i++) {
    if (s_arr[i] == ptr) {
      // APP_LOG(APP_LOG_LEVEL_INFO, "d");
      gbitmap_destroy(s_arr[i]);
      s_arr[i] = NULL;
      s_res_ids[i] = -1;
      return;
    }
  }

  // APP_LOG(APP_LOG_LEVEL_ERROR, "Bitmap ptr not found");
}

void bitmaps_destroy_id(uint32_t res_id) {
  for (int i = 0; i < MAX_BITMAPS; i++) {
    if (s_res_ids[i] == res_id) {
      // APP_LOG(APP_LOG_LEVEL_INFO, "d");
      gbitmap_destroy(s_arr[i]);
      s_arr[i] = NULL;
      s_res_ids[i] = -1;
      return;
    }
  }

  // APP_LOG(APP_LOG_LEVEL_ERROR, "Bitmap id not found");
}

void bitmaps_destroy_all() {
  for (int i = 0; i < MAX_BITMAPS; i++) {
    if (s_arr[i] != NULL) {
      gbitmap_destroy(s_arr[i]);
      s_arr[i] = NULL;
      s_res_ids[i] = -1;
    }
  }

  // APP_LOG(APP_LOG_LEVEL_INFO, "Bitmaps destroyed (heap %d)", heap_bytes_free());
}
