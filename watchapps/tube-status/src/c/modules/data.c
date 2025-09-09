#include "data.h"

static char s_line_states[LineTypeMax][32];
static char s_line_reasons[LineTypeMax][256];
static int s_progress = 0;

void data_init() {
}

void data_deinit() {
}

char* data_get_line_name(int type) {
  switch(type) {
    case LineTypeBakerloo:           return "Bakerloo";
    case LineTypeCentral:            return "Central";
    case LineTypeCircle:             return "Circle";
    case LineTypeDistrict:           return "District";
    case LineTypeDLR:                return "DLR";
    case LineTypeElizabeth:          return "Elizabeth";
    case LineTypeHammersmithAndCity: return "H'smith & City";
    case LineTypeJubilee:            return "Jubilee";
    case LineTypeLiberty:            return "Liberty";
    case LineTypeLioness:            return "Lioness";
    case LineTypeMetropolitan:       return "Metropolitan";
    case LineTypeMildmay:            return "Mildmay";
    case LineTypeNorthern:           return "Northern";
    case LineTypePicadilly:          return "Picadilly";
    case LineTypeSuffragette:        return "Suffragette";
    case LineTypeVictoria:           return "Victoria";
    case LineTypeWaterlooAndCity:    return "W'loo & City";
    case LineTypeWeaver:             return "Weaver";
    case LineTypeWindrush:           return "Windrush";
    default:                         return "?";
  }
}

char* data_get_line_state(int type) {
  return &s_line_states[type][0];
}

char* data_get_line_reason(int type) {
  return &s_line_reasons[type][0];
}

GColor data_get_line_color(int type) {
#if defined(PBL_COLOR)
  switch(type) {
    case LineTypeBakerloo:           return GColorFromHEX(0xB36305);
    case LineTypeCentral:            return GColorFromHEX(0xE32017);
    case LineTypeCircle:             return GColorFromHEX(0xFFD300);
    case LineTypeDistrict:           return GColorFromHEX(0x00782A);
    case LineTypeDLR:                return GColorFromHEX(0x00AFAD);
    case LineTypeElizabeth:          return GColorFromHEX(0x9364CD);
    case LineTypeHammersmithAndCity: return GColorFromHEX(0xF3A9BB);
    case LineTypeJubilee:            return GColorFromHEX(0xA0A5A9);
    case LineTypeLiberty:            return GColorFromHEX(0x686868);
    case LineTypeLioness:            return GColorFromHEX(0xFEAF3F);
    case LineTypeMetropolitan:       return GColorFromHEX(0x9B0056);
    case LineTypeMildmay:            return GColorCobaltBlue;
    case LineTypeNorthern:           return GColorBlack;
    case LineTypePicadilly:          return GColorFromHEX(0x003688);
    case LineTypeSuffragette:        return GColorFromHEX(0x00BA80);
    case LineTypeWaterlooAndCity:    return GColorFromHEX(0x95CDBA);
    case LineTypeWeaver:             return GColorFromHEX(0xA12860);
    case LineTypeWindrush:           return GColorFolly;
    case LineTypeVictoria:           return GColorFromHEX(0x0098D4);
    default:                         return GColorWhite;
  }
#endif
  return GColorBlack;
}

GColor data_get_line_state_color(int type) {
  char *state = s_line_states[type];
  
  // Minor, Part
  if(strstr(state, "inor") || strstr(state, "art")) {
    return PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorDarkGray);
  }

  // Severe, Planned, Closed, Suspended
  if(strstr(state, "evere") || strstr(state, "lanned") || strstr(state, "losed") || strstr(state, "uspended")) {
    return PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorDarkGray);
  }

  return GColorClear;
}

bool data_get_line_color_is_striped(int type) {
  switch(type) {
    case LineTypeDLR:
    case LineTypeElizabeth:
    case LineTypeLiberty:
    case LineTypeLioness:
    case LineTypeMildmay:
    case LineTypeSuffragette:
    case LineTypeWeaver:
    case LineTypeWindrush:
      return true;
    default:
      return false;
  }
}

void data_set_progress(int progress) {
  s_progress = progress;
}

int data_get_progress() {
  return s_progress;
}
