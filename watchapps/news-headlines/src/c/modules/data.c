#include "data.h"

// Paving the way for modular apps!

static Story s_stories[DATA_MAX_STORIES];
static GBitmap *s_thumb_bitmap = NULL;

static int s_quantity, s_downloaded, s_progress;
static bool s_thumb_valid;
static uint8_t *s_img_buffer;

void data_init() {
  for(int i = 0; i < DATA_MAX_STORIES; i++) {
    s_stories[i].valid = false;
  }

#if defined(PBL_COLOR)
  s_thumb_bitmap = gbitmap_create_blank(GSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT), GBitmapFormat8Bit);
  s_img_buffer = malloc(THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * sizeof(uint8_t));
#endif
}

Story* data_get_story(int index) {
  return &s_stories[index];
}

void data_set_quantity(int quantity) {
  s_quantity = quantity;
}

int data_get_quantity() {
  return s_quantity;
}

bool data_is_valid() {
  return s_stories[0].valid;
}

int data_get_downloaded() {
  return s_downloaded;
}

void data_store_story(DictionaryIterator *iter, int index) {
  Tuple *title_tuple = dict_find(iter, AppKeyTitle);
  Tuple *desc_tuple = dict_find(iter, AppKeyDescription);

  Story *story = data_get_story(index);
  story->valid = true;
  snprintf(story->title, DATA_MAX_LENGTH_TITLE, "%s", title_tuple->value->cstring);
  snprintf(story->description, DATA_MAX_LENGTH_DESC, "%s", desc_tuple->value->cstring);

  if(SHOW_LOGS) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got story %d", index);
    APP_LOG(APP_LOG_LEVEL_INFO, ">> %s", story->title);
    APP_LOG(APP_LOG_LEVEL_INFO, ">> %s", story->description);
  }

  s_downloaded = index;
}

void data_cache_data() {
  if(!persist_exists(DATA_PERSIST_KEY_CACHED)) {
    // This is the first cache
    persist_write_bool(DATA_PERSIST_KEY_CACHED, true);
  }

  for(int i = 0; i < DATA_MAX_PERSISTED; i++) {
    Story *s = data_get_story(i);
    persist_write_string(DATA_PERSIST_KEY_TITLE_BASE + i, s->title);
    persist_write_string(DATA_PERSIST_KEY_DESC_BASE + i, s->description);

    if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Cached item %d", i);
  }
}

bool data_load_cached_data() {
  for(int i = 0; i < DATA_MAX_PERSISTED; i++) {
    if(persist_exists(DATA_PERSIST_KEY_TITLE_BASE + i)) {
      Story *s = data_get_story(i);
      int len = persist_read_string(DATA_PERSIST_KEY_TITLE_BASE + i, s->title, DATA_MAX_PERSISTED_TITLE_LENGTH);  // handy!
      persist_read_string(DATA_PERSIST_KEY_DESC_BASE + i, s->description, DATA_MAX_PERSISTED_DESC_LENGTH);
      s->valid = true;

      // It may be truncated due to 256k max data length...
      memset(&s->title[len - 1], '.', 1);
      memset(&s->title[len - 2], '.', 1);
      memset(&s->title[len - 3], '.', 1);
    } else {
      // Cache ends prematurely
      data_set_quantity(i);
      s_downloaded = i;

      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Cache ended early at %d", i);
      return false;
    }
  }

  // All here!
  s_downloaded = DATA_MAX_PERSISTED - 1;
  data_set_quantity(DATA_MAX_PERSISTED);

  return true;
}

int data_checksum(int index) {
#if defined(PBL_COLOR)
  int result = 0;
  for(uint32_t j = 0; j < strlen(s_stories[index].title); j++) {
    result += s_stories[index].title[j];
  }
  if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Generated checksum: %d", result);
  return result;
#else
  return 0;
#endif
}

GBitmap* data_get_thumbnail() {
#if defined(PBL_COLOR)
  return s_thumb_valid ? s_thumb_bitmap : NULL;
#else
  return NULL;
#endif
}

void data_set_thumbnail_valid(bool b) {
#if defined(PBL_COLOR)
  s_thumb_valid = b;

  if(b) {
    gbitmap_set_data(s_thumb_bitmap, s_img_buffer, GBitmapFormat8Bit, THUMBNAIL_WIDTH, false);
  }
#endif
}

uint8_t* data_get_buffer() {
#if defined(PBL_COLOR)
  return s_img_buffer;
#else
  return NULL;
#endif
}

void data_set_thumbnail_progress(int progress) {
#if defined(PBL_COLOR)
  s_progress = progress;

  if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Thumb progress: %d", s_progress);
#endif
}

int data_get_thumbnail_progress() {
#if defined(PBL_COLOR)
  return s_progress;
#else
  return 0;
#endif
}

void data_deinit() {
#if defined(PBL_COLOR)
  if(s_thumb_bitmap) {
    gbitmap_destroy(s_thumb_bitmap);
  }
  if(s_img_buffer) {
    free(s_img_buffer);
  }
#endif
}

void data_story_set_has_image(int index, bool has_image) {
  s_stories[index].has_image = has_image;
}

bool data_story_has_image(int index) {
  return s_stories[index].has_image;
}
