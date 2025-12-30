#include <pebble.h>
#include <pebble-packet/pebble-packet.h>

static Window *s_window;
static TextLayer *s_text_layer;

static void failed_handler(void) {
  text_layer_set_text(s_text_layer, "Failed to send packet");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Sending");

  if (packet_begin()) {
    packet_put_string(MESSAGE_KEY_OUTBOUND, "Example message");
    packet_send(failed_handler);

    text_layer_set_text(s_text_layer, "Packet sent!");
  } else {
    text_layer_set_text(s_text_layer, "Failed to create packet");
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Press select button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox received with size %d", packet_get_size(iter));

  if (packet_contains_key(iter, MESSAGE_KEY_INBOUND)) {
    const char *message = packet_get_string(iter, MESSAGE_KEY_INBOUND);
    text_layer_set_text(s_text_layer, message);
  } else {
    text_layer_set_text(s_text_layer, "No inbound message");
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped with reason %d", reason);
  text_layer_set_text(s_text_layer, "Inbox dropped");
}

static void init(void) {
  packet_init();

  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_register_inbox_received(inbox_received_handler);
  const AppMessageResult r = app_message_open(256, 256);
  if (r != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open AppMessage: %s", packet_result_to_string(r));
  }

  s_window = window_create();
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
