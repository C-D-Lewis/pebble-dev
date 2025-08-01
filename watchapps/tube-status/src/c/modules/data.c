#include "data.h"

static char s_line_states[LineTypeMax][32];

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
    case LineTypeHammersmithAndCity: return "H'smith & City";
    case LineTypeJubilee:            return "Jubilee";
    case LineTypeMetropolitan:       return "Metropolitan";
    case LineTypeNorthern:           return "Northern";
    case LineTypePicadilly:          return "Picadilly";
    case LineTypeVictoria:           return "Victoria";
    case LineTypeWaterlooAndCity:    return "W'loo & City";
    default:                         return "UNKNOWN LINE!";
  }
}

char* data_get_line_state(int type) {
  return &s_line_states[type][0];
}

GColor data_get_line_color(int type) {
#if defined(PBL_COLOR)
  switch(type) {
    case LineTypeBakerloo:           return GColorFromHEX(0xB36305);
    case LineTypeCentral:            return GColorFromHEX(0xE32017);
    case LineTypeCircle:             return GColorFromHEX(0xFFD300);
    case LineTypeDistrict:           return GColorFromHEX(0x00782A);
    case LineTypeHammersmithAndCity: return GColorFromHEX(0xF3A9BB);
    case LineTypeJubilee:            return GColorFromHEX(0xA0A5A9);
    case LineTypeMetropolitan:       return GColorFromHEX(0x9B0056);
    case LineTypeNorthern:           return GColorBlack;
    case LineTypePicadilly:          return GColorFromHEX(0x003688);
    case LineTypeVictoria:           return GColorFromHEX(0x0098D4);
    case LineTypeMax:                return GColorDukeBlue;
    case LineTypeWaterlooAndCity:    return GColorTiffanyBlue;
    default:  break;
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

  // Sever, Planned, Closed
  if(strstr(state, "evere") || strstr(state, "lanned") || strstr(state, "losed")) {
    return PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorDarkGray);
  }

  return GColorClear;
}
