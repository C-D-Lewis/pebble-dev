#include "comm.h"

static AppTimer *s_timeout_timer;
static bool s_quick_started = false;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  if(s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }

  int size = (int)iter->end - (int)iter->dictionary;
  if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Dict size: %d", size);

  // Wrong version gate
  if(dict_find(iter, ErrorCodeWrongVersion) != NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Wrong version error!");

    dialog_window_push(
      PBL_IF_ROUND_ELSE("Update\nwatchapp using the Dashboard Android app!",
                        "Update watchapp using the Dashboard Android app!")
    );
    splash_window_remove_from_stack();
    return;
  }

  // Process the incoming update
  if(dict_find(iter, MessageTypeRequestAll) != NULL) {
    data_set_gsm_name(dict_find(iter, AppKeyGSMName)->value->cstring);
    data_set_wifi_name(dict_find(iter, AppKeyWifiName)->value->cstring);
    data_set_gsm_percent(dict_find(iter, AppKeyGSMPercent)->value->int8);
    data_set_battery_percent(dict_find(iter, AppKeyBatteryPercent)->value->int8);
    data_set_storage_gb_major(dict_find(iter, AppKeyStorageFreeGBMajor)->value->int8);
    data_set_storage_gb_minor(dict_find(iter, AppKeyStorageFreeGBMinor)->value->int8);
    data_set_storage_percent(dict_find(iter, AppKeyStoragePercent)->value->int8);
    data_set_toggle_order(dict_find(iter, AppKeyToggleOrderString)->value->cstring);
    data_set_is_lollipop(dict_find(iter, AppKeyIsLollipop)->value->int8 == 1);

    // 'Request all' response
    data_set_toggle_state(ToggleTypeWifi, (dict_find(iter, AppKeyToggleWifi)->value->int8 == 1) ? ToggleStateOn : ToggleStateOff);
    data_set_toggle_state(ToggleTypeData, (dict_find(iter, AppKeyToggleData)->value->int8 == 1) ? ToggleStateOn : ToggleStateOff);
    data_set_toggle_state(ToggleTypeBluetooth, (dict_find(iter, AppKeyToggleBluetooth)->value->int8 == 1) ? ToggleStateOn : ToggleStateOff);
    data_set_toggle_state(ToggleTypeWifiAP, (dict_find(iter, AppKeyToggleWifiAP)->value->int8 == 1) ? ToggleStateOn : ToggleStateOff);
    data_set_toggle_state(ToggleTypeSync, (dict_find(iter, AppKeyToggleSync)->value->int8 == 1) ? ToggleStateOn : ToggleStateOff);
    data_set_toggle_state(ToggleTypeFindPhone, (dict_find(iter, AppKeyToggleFindPhone)->value->int8 == 1) ? ToggleStateOn : ToggleStateOff);
    data_set_toggle_state(ToggleTypeRinger, dict_find(iter, AppKeyToggleRinger)->value->int8); // Uses toggle state in encoding
    data_set_toggle_state(ToggleTypeAutoBrightness, dict_find(iter, AppKeyToggleAutoBrightness)->value->int8);
    data_set_toggle_state(ToggleTypeLockPhone, ToggleStateOff); // Always

    unified_window_push();
    unified_window_set_interaction_enabled(true);
    splash_window_remove_from_stack();

    // Quick launch?
    Tuple *quick_launch_enabled_t = dict_find(iter, AppKeyQuickLaunchEnabled);
    if(quick_launch_enabled_t) {
      bool enabled = quick_launch_enabled_t->value->int8 == 1;
      int type = dict_find(iter, AppKeyQuickLaunchType)->value->int8;

      if(enabled && !s_quick_started) {
        s_quick_started = true;
        unified_window_jump_to(type);
      }
    }
  } 

  // 'Toggle' response
  else {
    // If response for WiFi AP, request another full sync
    for(int key = AppKeyToggleWifi; key < AppKeyToggleOrderString; key++) {   // Flaky if something extends the linear list of keys here
      Tuple *t = dict_find(iter, key);
      if(t) {
        int type = key - AppKeyToggleWifi;  // Also enum dependent
        int8_t new_state = t->value->int8;
        if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Received type %d set to %d", type, new_state);

        switch(type) {
          // new state encoded
          case ToggleTypeData:
            if(new_state == ErrorCodeNoRoot) {
              dialog_window_push(PBL_IF_ROUND_ELSE(
                "Data toggle\nnot available without root.",
                "Data toggle not available without root."));
              data_set_toggle_state(ToggleTypeData, ToggleStateOff);
            } else if(new_state == ErrorCodeDataNotEnabled) {
              dialog_window_push(PBL_IF_ROUND_ELSE(
                "Data toggle\nnot enabled in Android app.",
                "Data toggle not enabled in Android app."));
              data_set_toggle_state(ToggleTypeData, ToggleStateOff);
            }
            break;
          case ToggleTypeWifi:
          case ToggleTypeSync:
          case ToggleTypeWifiAP:
          case ToggleTypeFindPhone:
          case ToggleTypeAutoBrightness:
          case ToggleTypeRinger:
            data_set_toggle_state(type, new_state);
            break;
          // Off only
          case ToggleTypeBluetooth:
            dialog_window_push("Phone is now disconnected.");
            unified_window_remove_from_stack();
            splash_window_remove_from_stack();
            data_set_toggle_state(type, ToggleStateOff);
            break;
          case ToggleTypeLockPhone:
            if(new_state == ErrorCodeNoDeviceAdmin) {
              dialog_window_push("Unavailable without Device Admin permission.");
              data_set_toggle_state(type, ToggleStateOff);
            } else if(new_state == ErrorCodeLockSuccess) {
              data_set_toggle_state(type, ToggleStateLocked);
            }
            break;
        }
      } else {
        if(DEBUG) APP_LOG(APP_LOG_LEVEL_ERROR, "AppKey %d was not present", key);
      }
    }
  }

  unified_window_reload();
}

static void timeout_handler(void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Timed out. Retrying...");
  comm_request_all();
}

static void init_timeout_timer() {
  s_timeout_timer = app_timer_register(COMM_TIMEOUT_MS, timeout_handler, NULL);
}

bool comm_check_bt(char *failed_notice) {
  if(!connection_service_peek_pebble_app_connection()) {
    dialog_window_push(failed_notice);
    splash_window_remove_from_stack();
    unified_window_remove_from_stack();
    return false;
  } else {
    return true;
  }
}

void comm_request_toggle(int type, int state) {
  if(TEST) {
    dialog_window_push("Cannot toggle in test mode");
    return;
  }

  if(!comm_check_bt("Watch disconnected. Reconnect to continue.")) {
    return;
  }

  // Transform type from type enum value to key enum value (they are linear)
  type += AppKeyToggleWifi;

  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open outbox: %d", (int)result);
  }

  const int dummy = 0;
  dict_write_int(iter, MessageTypeToggle, &dummy, sizeof(int), true);
  dict_write_int(iter, type, &state, sizeof(int), true);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox: %d", (int)result);
  }
}

void comm_request_all() {
  if(TEST) {
    return;
  }

  if(!comm_check_bt("Watch disconnected. Reconnect to continue.")) {
    return;
  }

  // Just in case, but first time should be immediately after version check
  app_message_register_inbox_received(inbox_received_handler);

  // Can't use pebble-packet due to low memory on Aplite
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open outbox: %d", (int)result);
    return;
  }

  const int dummy = 0;
  dict_write_int(iter, MessageTypeRequestAll, &dummy, sizeof(int), true);

  // Add version instead of version_check library
  dict_write_cstring(iter, AppKeyVersion, VERSION);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox: %d", (int)result);
  } else {
    init_timeout_timer();
  }
}

void comm_init(uint32_t inbox, uint32_t outbox) {
  app_message_open(inbox, outbox);
}
