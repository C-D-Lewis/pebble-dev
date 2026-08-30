#include "util.h"

char* util_get_toggle_name(int index) {
  switch(index) {
    case ToggleTypeFindPhone: return "Find Phone";
    case ToggleTypeCycleRinger: {
      const char c = data_get_toggle_state(index);
      switch(c) {
        case TOGGLE_STATE_RINGER_LOUD: return "Ringer (loud)";
        case TOGGLE_STATE_RINGER_VIBRATE: return "Ringer (vibrate)";
        case TOGGLE_STATE_RINGER_DND: return "Ringer (silent)";
        default: return "Ringer (?)";
      }
    } break;
    default: return "?";
  }
}

uint32_t util_get_toggle_res_id(int index) {
  switch(index) {
    case ToggleTypeFindPhone: return RESOURCE_ID_PHONE;
    case ToggleTypeCycleRinger: {
      const char c = data_get_toggle_state(index);
      switch(c) {
        case TOGGLE_STATE_RINGER_LOUD: return RESOURCE_ID_LOUD;
        case TOGGLE_STATE_RINGER_VIBRATE: return RESOURCE_ID_VIBRATE;
        case TOGGLE_STATE_RINGER_DND: return RESOURCE_ID_SILENT;
        default: return RESOURCE_ID_NOTHING;
      }
    }
    default: return RESOURCE_ID_NOTHING;
  }
}