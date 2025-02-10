#pragma once

#include <pebble.h>

#include "../config.h"
#include "comm.h"

#define DATA_MAX_STORIES      20
#define DATA_MAX_LENGTH_TITLE 128
#define DATA_MAX_LENGTH_DESC  256

#define DATA_MAX_PERSISTED              10   // Maximum number of cached stories
#define DATA_PERSIST_KEY_CACHED         0    // Is there cached data?
#define DATA_PERSIST_KEY_TITLE_BASE     100  // 100 + i
#define DATA_PERSIST_KEY_DESC_BASE      200  // 200 + i
#define DATA_MAX_PERSISTED_TITLE_LENGTH 50   // 50 + 200 == PERSIST_DATA_MAX_LENGTH
#define DATA_MAX_PERSISTED_DESC_LENGTH  200

typedef struct {
  bool valid;
  char title[DATA_MAX_LENGTH_TITLE];
  char description[DATA_MAX_LENGTH_DESC];
  bool has_image;
} Story;

// Initialize data stores
void data_init();

// Deinit gbitmap
void data_deinit();

// Get a pointer to a Story object
Story* data_get_story(int index);

// Set the quantity of stories available
void data_set_quantity(int quantity);

// Get the quantity of stories available
int data_get_quantity();

// Check at least one story has been received
bool data_is_valid();

// Get the number of downloaded stories
int data_get_downloaded();

// Store a received story in the data store
void data_store_story(DictionaryIterator *iter, int index);

// Cache the first 10 stories in persistent storage
void data_cache_data();

// Load the last stored 10 stories from persistent storage
bool data_load_cached_data();

// Checksum a story's title
int data_checksum(int index);

// Get the current image thumbnail
GBitmap* data_get_thumbnail();

// Mark validity of thumbnail
void data_set_thumbnail_valid(bool b);

// Get the GBitmap buffer
uint8_t* data_get_buffer();

// Set the thumbnail progress
void data_set_thumbnail_progress(int progress);

// Get the thumbnail progress
int data_get_thumbnail_progress();

// Set if this story has an image
void data_story_set_has_image(int index, bool has_image);

// Get if the story has an image available
bool data_story_has_image(int index);
