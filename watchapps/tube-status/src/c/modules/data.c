#include "data.h"

static LineData s_line_data[LineTypeMax];
static int s_progress = 0;
static int s_progress_max = LineTypeMax;

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
    case LineTypeSuffragette:        return GColorMayGreen;
    case LineTypeWaterlooAndCity:    return GColorFromHEX(0x95CDBA);
    case LineTypeWeaver:             return GColorFromHEX(0xA12860);
    case LineTypeWindrush:           return GColorFromHEX(0xE32017);
    case LineTypeVictoria:           return GColorFromHEX(0x0098D4);
    default:                         return GColorWhite;
  }
#endif
  return GColorBlack;
}

LineData* data_get_line(int index) {
  return &s_line_data[index];
}

GColor data_get_line_state_color(int index) {
  char *state = data_get_line(index)->state;
  
  // Minor, Part
  if (strstr(state, "inor") || strstr(state, "art")) {
    return PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorDarkGray);
  }

  // Severe, Planned, Closed, Suspended
  if (strstr(state, "evere") || strstr(state, "lanned") || strstr(state, "losed") || strstr(state, "uspended")) {
    return PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray);
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

bool data_get_line_has_reason(int index) {
  char *reason = data_get_line(index)->reason;
  return strlen(reason) != 0;
}

void data_set_progress_max(int max) {
  s_progress_max = max;
}

int data_get_progress_max() {
  return s_progress_max;
}

int data_get_lines_received() {
  int count = 0;
  for (int i = 0; i < LineTypeMax; i++) {
    if (strlen(s_line_data[i].state) != 0) {
      count++;
    }
  }
  return count;
}
