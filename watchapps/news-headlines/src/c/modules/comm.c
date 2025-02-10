#include "comm.h"

static AppTimer *s_timeout_timer = NULL, *s_image_timeout_timer = NULL;

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  Tuple *t = NULL;

  // JS Ready
  t = dict_find(iter, AppKeyReady);
  if(t) {
    splash_window_begin();
  }

  // Quantity?
  t = dict_find(iter, AppKeyQuantity);
  if(t) {
    int quantity = t->value->int32;
    data_set_quantity(quantity);

    splash_window_set_progress(0);

    if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Got quantity: %d", data_get_quantity());

    // Image availability - relies on having gotten quantity
    t = dict_find(iter, AppKeyImageAvailabilityString);
    if(t) {
      char *string = t->value->cstring;

      for(int i = 0; i < quantity; i++) {
        data_story_set_has_image(i, string[i] == '1');
      }
    }
  }

  //  Story
  t = dict_find(iter, AppKeyIndex);
  if(t) {
    int index = t->value->int32;
    data_store_story(iter, index);

    splash_window_set_progress(index);
  }

  // Server status result
  t = dict_find(iter, AppKeyStatus);
  if(t) {
    settings_window_update_pin_server_status((ServerStatus)(int)t->value->int32);

    // Cancel timeout
    if(s_timeout_timer) {
      app_timer_cancel(s_timeout_timer);
    }
  }

  // Image data?
  t = dict_find(iter, AppKeyOffset);
  if(t) {
    int offset = t->value->int32;
    int size = THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT;

    if(s_image_timeout_timer) {
      app_timer_cancel(s_image_timeout_timer);
      s_image_timeout_timer = NULL;
    }

    // Store data package
    Tuple *len_t = dict_find(iter, AppKeyChunkSize);
    int length = 0;
    if(len_t) {
      length = len_t->value->int32;
      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Chunk length: %d", length);
    } else {
      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "CHUNK LENGTH IS ZERO!");
      return;
    }

    t = dict_find(iter, AppKeyData);
    if(t && (offset + length) <= size) {
      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Got offset %d with len %d", offset, length);
      uint8_t *data = data_get_buffer();
      if(data) {
        memcpy(&data[offset], t->value->data, length);

        int progress = (offset * 100) / size;
        data_set_thumbnail_progress(progress);
        image_window_redraw_thumbnail();
      }
    } else {
      if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "Offset+length %d is greater than size %d", offset + length, size);
    }
  }

  // Image fetch done!
  t = dict_find(iter, AppKeyImageDone);
  if(t) {
    if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Image fetch complete!");
    data_set_thumbnail_valid(true);
    image_window_redraw_thumbnail();
    comm_set_fast(false);
  }

  // Image fetch failed
  t = dict_find(iter, AppKeyImageFailed);
  if(t) {
    if(SHOW_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to fetch story image");
    comm_set_fast(false);
    data_set_thumbnail_valid(false);
    image_window_redraw_thumbnail();
    data_set_thumbnail_progress(-1);
  }
}

void comm_init() {
  const int inbox_size = 2026; // For image chunk size
  const int outbox_size = 656;
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(inbox_size, outbox_size);
  comm_set_fast(true);
}

void comm_request_quantity() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  int dummy = 0;
  dict_write_int(iter, AppKeyQuantity, &dummy, sizeof(int), true);

  app_message_outbox_send();
}

void comm_send_settings() {
  DictionaryIterator *out;
  app_message_outbox_begin(&out);

  int value = settings_get_category();
  dict_write_int(out, AppKeySettingsCategory, &value, sizeof(int), true);
  value = (int)settings_get_subscribed_type();
  dict_write_int(out, AppKeySettingsSubscription, &value, sizeof(int), true);
  value = settings_get_number_of_stories();
  dict_write_int(out, AppKeySettingsNumStories, &value, sizeof(int), true);
  value = settings_get_region();
  dict_write_int(out, AppKeySettingsRegion, &value, sizeof(int), true);

  app_message_outbox_send();
}

static void timeout(void *context) {
  settings_window_update_pin_server_status(ServerStatusTimeout);

  s_timeout_timer = NULL;
}

void comm_get_pin_server_status() {
  DictionaryIterator *out;
  app_message_outbox_begin(&out);

  int dummy = 0;
  dict_write_int(out, AppKeyStatus, &dummy, sizeof(int), true);
  
  app_message_outbox_send();
  s_timeout_timer = app_timer_register(COMM_TIMEOUT_MS, timeout, NULL);
}

void comm_set_fast(bool fast) {
  app_comm_set_sniff_interval(fast ? SNIFF_INTERVAL_REDUCED: SNIFF_INTERVAL_NORMAL);
}

static void image_timeout(void *context) {
#if defined(PBL_COLOR)
  comm_set_fast(false);
  s_image_timeout_timer = NULL;
#endif
}

void comm_get_thumbnail(int index) {
#if defined(PBL_COLOR)
  // Prepare
  comm_set_fast(true);
  data_set_thumbnail_valid(false);
  data_set_thumbnail_progress(0);

  // Send
  DictionaryIterator *out;
  app_message_outbox_begin(&out);
  int value = settings_get_category();
  dict_write_int(out, AppKeySettingsCategory, &value, sizeof(int), true);
  int hash = data_checksum(index);
  dict_write_int(out, AppKeyImage, &hash, sizeof(int), true);
  app_message_outbox_send();

  s_image_timeout_timer = app_timer_register(COMM_TIMEOUT_MS, image_timeout, NULL);
#endif
}
