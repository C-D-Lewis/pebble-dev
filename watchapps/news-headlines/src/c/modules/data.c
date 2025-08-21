#include "data.h"

static Story s_stories[DATA_MAX_STORIES];
static int s_quantity, s_downloaded;

void data_init() {
  for(int i = 0; i < DATA_MAX_STORIES; i++) {
    s_stories[i].valid = false;
  }
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

  if (SHOW_LOGS) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got story %d", index);
    APP_LOG(APP_LOG_LEVEL_INFO, ">> %s", story->title);
    APP_LOG(APP_LOG_LEVEL_INFO, ">> %s", story->description);
  }

  s_downloaded = index;
}

void data_cache_data() {
  if (!persist_exists(DATA_PERSIST_KEY_CACHED)) {
    // This is the first cache
    persist_write_bool(DATA_PERSIST_KEY_CACHED, true);
  }

  for(int i = 0; i < DATA_MAX_PERSISTED; i++) {
    Story *s = data_get_story(i);
    persist_write_string(DATA_PERSIST_KEY_TITLE_BASE + i, s->title);
    persist_write_string(DATA_PERSIST_KEY_DESC_BASE + i, s->description);

    if (SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Cached item %d", i);
  }
}

bool data_load_cached_data() {
  for(int i = 0; i < DATA_MAX_PERSISTED; i++) {
    if (persist_exists(DATA_PERSIST_KEY_TITLE_BASE + i)) {
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

      if (SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Cache ended early at %d", i);
      return false;
    }
  }

  // All here!
  s_downloaded = DATA_MAX_PERSISTED - 1;
  data_set_quantity(DATA_MAX_PERSISTED);

  return true;
}

void data_deinit() {
}
