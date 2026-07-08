#include "util.h"

char* util_get_toggle_name(int index) {
  // 0 is nothing
  index++;

  switch(index) {
    case ToggleTypeAutoSync: return "AutoSync";
    default: return "?";
  }
}

uint32_t util_get_toggle_res_id(int index) {
  // 0 is nothing
  index++;

  switch(index) {
    case ToggleTypeAutoSync: return RESOURCE_ID_PHONE;
    default: return RESOURCE_ID_NOTHING;
  }
}