#include "pge_ws.h"

static PGEWSConnectedHandler *s_connection_handler;
static PGEWSReceivedHandler *s_recv_handler;

static PGEWSConnectionState s_connection_state = PGEWSConnectionStateDisconnected;
static DictionaryIterator *s_outbox_iter, *s_inbox_iter;
static int s_client_id;
static bool s_app_message_open, s_js_ready;
static char *s_url_ptr;

static void in_recv_handler(DictionaryIterator *iter, void *context) {
  // Probably temporary. Used to get data in s_recv_handler
  s_inbox_iter = iter;

  // Was the connection successful?
  Tuple *tuple = dict_find(iter, PGE_WS_URL);
  if(tuple) {
    s_connection_state = (tuple->value->int32 == 1) ? PGEWSConnectionStateConnected : PGEWSConnectionStateDisconnected;
    if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_DEBUG, "PGE_WS: Connection result: %s", s_connection_state ? "OK" : "FAILED");

    // Store client ID in this bundle
    Tuple *id_tuple = dict_find(iter, PGE_WS_CLIENT_ID);
    if(id_tuple) {
      s_client_id = id_tuple->value->int32;
      if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "Got ID %d", s_client_id);
    } else {
      s_connection_state = PGEWSConnectionStateDisconnected;
      if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "CLient ID was not provided by the server.");
    }

    // Call the developer callback
    s_connection_handler(s_connection_state == PGEWSConnectionStateConnected);
  }

  // JS Ready event?
  tuple = dict_find(iter, PGE_WS_READY);
  if(tuple) {
    s_js_ready = true;

    // Send URL to JS, wait for connected callback
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_cstring(iter, PGE_WS_URL, s_url_ptr);
    app_message_outbox_send();
    if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_DEBUG, "PGE_WS: URL sent");
  }

  // Developer-implemented keys? Callback if at least one. Use pge_ws_get_value() to get values
  for(int i = 0; i < PGE_WS_NUM_KEYS; i++) {
    tuple = dict_find(iter, i);
    if(tuple) {
      s_recv_handler();  // Use pge_ws_get_value() in here
      break;
    }
  }

  // Iterator expires here
  s_inbox_iter = NULL;
}

static void parse_result(AppMessageResult result) {
  switch(result) {
    case APP_MSG_OK: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_OK"); break;
    case APP_MSG_SEND_TIMEOUT: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_SEND_TIMEOUT"); break;
    case APP_MSG_SEND_REJECTED: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_SEND_REJECTED"); break;
    case APP_MSG_NOT_CONNECTED: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_NOT_CONNECTED"); break;
    case APP_MSG_APP_NOT_RUNNING: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_APP_NOT_RUNNING"); break;
    case APP_MSG_INVALID_ARGS: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_INVALID_ARGS"); break;
    case APP_MSG_BUSY: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_BUSY"); break;
    case APP_MSG_BUFFER_OVERFLOW: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_BUFFER_OVERFLOW"); break;
    case APP_MSG_ALREADY_RELEASED: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_ALREADY_RELEASED"); break;
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_CALLBACK_ALREADY_REGISTERED"); break;
    case APP_MSG_CALLBACK_NOT_REGISTERED: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_CALLBACK_NOT_REGISTERED"); break;
    case APP_MSG_OUT_OF_MEMORY: APP_LOG(APP_LOG_LEVEL_DEBUG, "APP_MSG_OUT_OF_MEMORY"); break;
    default: break;
  }
}

void pge_ws_begin(char *url, PGEWSConnectedHandler *handler, PGEWSReceivedHandler *recv_handler) {
  if(s_connection_state == PGEWSConnectionStateDisconnected) {
    // Set up callbacks
    s_connection_handler = handler;
    s_recv_handler = recv_handler;
    s_url_ptr = url;

    // Prepare to receive, wait for ready
    if(!s_app_message_open) {
      s_app_message_open = true;
      app_message_register_inbox_received(in_recv_handler);
      app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
      app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
      if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_DEBUG, "PGE_WS: AppMessage opened");
    }
  } else {
    if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_INFO, "PGE_WS: Already connected, or connection in progress!");
  }
}

bool pge_ws_is_connected() {
  return s_connection_state == PGEWSConnectionStateConnected;
}

int pge_ws_get_client_id() {
  return s_client_id;
}

bool pge_ws_packet_begin() {
  AppMessageResult result = app_message_outbox_begin(&s_outbox_iter);
  if(result != APP_MSG_OK) {
    parse_result(result);
    return false;
  } else {
    return true;
  }
}

bool pge_ws_packet_send() {
  AppMessageResult result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    parse_result(result);
    return false;
  } else {
    s_outbox_iter = NULL;
    return true;
  }
}

bool pge_ws_add_int(int key, int value) {
  if(!s_outbox_iter) {
    if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "pge_ws_packet_begin() has not been called!");
    return false;
  }

  AppMessageResult result = dict_write_int(s_outbox_iter, key, &value, sizeof(int), true);
  if(result != APP_MSG_OK) {
    parse_result(result);
    return false;
  }

  return true;
}

int pge_ws_get_value(int key) {
  if(s_inbox_iter) {
    Tuple *tuple = dict_find(s_inbox_iter, key);
    if(tuple) {
      // Found it!
      return tuple->value->int32;
    }

    // Failed 
    return PGE_WS_NOT_FOUND;
  } else {
    if(PGE_WS_LOGS) APP_LOG(APP_LOG_LEVEL_ERROR, "pge_ws_get_value() must be called within a PGEWSReceivedHandler!");
    return PGE_WS_NOT_FOUND;
  }
}