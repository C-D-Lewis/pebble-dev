#include "util.h"

char* util_get_toggle_name(int index) {
  switch(index) {
    case ToggleTypeFindPhone: return "Find Phone";
    default: return "?";
  }
}

uint32_t util_get_toggle_res_id(int index) {
  switch(index) {
    case ToggleTypeFindPhone: return RESOURCE_ID_PHONE;
    default: return RESOURCE_ID_NOTHING;
  }
}