#include "icons.h"

#define NUM_ICONS 11

static GBitmap *s_bitmaps[NUM_ICONS];

void icons_load() {
  s_bitmaps[0] = gbitmap_create_with_resource(RESOURCE_ID_WIFI);
  s_bitmaps[1] = gbitmap_create_with_resource(RESOURCE_ID_DATA);
  s_bitmaps[2] = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH);
  s_bitmaps[3] = gbitmap_create_with_resource(RESOURCE_ID_RINGER_LOUD);
  s_bitmaps[4] = gbitmap_create_with_resource(RESOURCE_ID_RINGER_VIBRATE);
  
  if(data_get_is_lollipop()) {
    s_bitmaps[5] = gbitmap_create_with_resource(RESOURCE_ID_RINGER_PRIORITY);
  } else {
    s_bitmaps[5] = gbitmap_create_with_resource(RESOURCE_ID_RINGER_SILENT);
  }

  s_bitmaps[6] = gbitmap_create_with_resource(RESOURCE_ID_SYNC);
  s_bitmaps[7] = gbitmap_create_with_resource(RESOURCE_ID_WIFI_AP);
  s_bitmaps[8] = gbitmap_create_with_resource(RESOURCE_ID_FIND_PHONE);
  s_bitmaps[9] = gbitmap_create_with_resource(RESOURCE_ID_LOCK_PHONE);
  s_bitmaps[10] = gbitmap_create_with_resource(RESOURCE_ID_BRIGHTNESS);

  // Check integrity
  for(int i = 0; i < NUM_ICONS; i++) {
    if(s_bitmaps[i] == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to allocate bitmap %d", i);
    }
  }
}

void icons_unload() {
  for(int i = 0; i < NUM_ICONS; i++) {
    if(s_bitmaps[i]) {
      gbitmap_destroy(s_bitmaps[i]);
    }
  }
}

GBitmap* icons_get_with_type(int type) {
  switch(type) {
    case ToggleTypeWifi:           return s_bitmaps[0];
    case ToggleTypeData:           return s_bitmaps[1];
    case ToggleTypeBluetooth:      return s_bitmaps[2];
    case ToggleTypeRinger: {
      int ringer_state = data_get_toggle_state(ToggleTypeRinger);
      switch(ringer_state) {
        case ToggleStateLoud:      return s_bitmaps[3];
        case ToggleStateVibrate:   return s_bitmaps[4];
        case ToggleStateSilent:    return s_bitmaps[5];
        default: 
          APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown icons_get_with_type() for ringer state %d", ringer_state);
          return s_bitmaps[8];  // '?'
      }
    } break;
    case ToggleTypeSync:           return s_bitmaps[6];
    case ToggleTypeWifiAP:         return s_bitmaps[7];
    case ToggleTypeFindPhone:      return s_bitmaps[8];
    case ToggleTypeLockPhone:      return s_bitmaps[9];
    case ToggleTypeAutoBrightness: return s_bitmaps[10];

    default: 
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown icons_get_with_type() for type %d", type);
      return NULL;
  }
}
