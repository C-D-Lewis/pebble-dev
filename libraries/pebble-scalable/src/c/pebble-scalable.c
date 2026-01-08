#include "pebble-scalable.h"

///////////////////////////////////////////// Geometry /////////////////////////////////////////////

// A simple helper to pick the right member based on the build platform
#if defined(PBL_PLATFORM_CHALK)
  #define _GET_SV(s) ((s).c ? (s).c : (s).o)
#elif defined(PBL_PLATFORM_EMERY)
  #define _GET_SV(s) ((s).e ? (s).e : (s).o)
#else
  #define _GET_SV(s) ((s).o)
#endif

// Half the scale input in thousandths
#define _T_PERC_HALF_RANGE 500

static int scale(int t_perc, int dimension) {
  return ((t_perc * dimension) + _T_PERC_HALF_RANGE) / 1000;
}

int scalable_x(SV values) {
  return scale(_GET_SV(values), PS_DISP_W);
}

int scalable_y(SV values) {
  return scale(_GET_SV(values), PS_DISP_H);
}

GRect scalable_center_x(GRect r) {
  r.origin.x = (PS_DISP_W - r.size.w) / 2;
  return r;
}

GRect scalable_center_y(GRect r) {
  r.origin.y = (PS_DISP_H - r.size.h) / 2;
  return r;
}

GRect scalable_center(GRect r) {
  return scalable_center_x(scalable_center_y(r));
}

/////////////////////////////////////////////// Fonts //////////////////////////////////////////////

// Max font sets that can be stored
#define _MAX_FONT_SETS 16

// Macro to pick the pointer based on the build target
#if defined(PBL_PLATFORM_CHALK)
  #define GET_SF(s) ((s).c ? (s).c : (s).o)
#elif defined(PBL_PLATFORM_EMERY)
  #define GET_SF(s) ((s).e ? (s).e : (s).o)
#else
  #define GET_SF(s) ((s).o)
#endif

static GFont s_fonts_ptrs[_MAX_FONT_SETS];

void scalable_set_fonts(int size_id, SF fonts) {
  if (size_id >= _MAX_FONT_SETS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "ps: font id %d exceeds max", size_id);
    return;
  }

  // Pick the correct pointer now and store only that one
  s_fonts_ptrs[size_id] = GET_SF(fonts);
}

GFont scalable_get_font(int size_id) {
  if (size_id >= _MAX_FONT_SETS || s_fonts_ptrs[size_id] == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "ps: font %d not set", size_id);
    return NULL; 
  }

  return s_fonts_ptrs[size_id];
}
