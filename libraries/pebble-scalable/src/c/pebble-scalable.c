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
// Square root of 2 divided 2, multiplied by 1000
#define SQRT_2_OVER_2 707

static int scale(int t_perc, int dimension) {
  return ((t_perc * dimension) + _T_PERC_HALF_RANGE) / 1000;
}

int scl_x(int t_perc) {
  return scale(t_perc, PS_DISP_W);
}

int scl_y(int t_perc) {
  return scale(t_perc, PS_DISP_H);
}

int _scl_x_pp_impl(SV values) {
  return scale(_GET_SV(values), PS_DISP_W);
}

int _scl_y_pp_impl(SV values) {
  return scale(_GET_SV(values), PS_DISP_H);
}

GRect scl_grect(int x_t_perc, int y_t_perc, int w_t_perc, int h_t_perc) {
  return GRect(
    scale(x_t_perc, PS_DISP_W),
    scale(y_t_perc, PS_DISP_H),
    scale(w_t_perc, PS_DISP_W),
    scale(h_t_perc, PS_DISP_H)
  );
}

GRect scl_center_x(GRect r) {
  r.origin.x = (PS_DISP_W - r.size.w) / 2;
  return r;
}

GRect scl_center_y(GRect r) {
  r.origin.y = (PS_DISP_H - r.size.h) / 2;
  return r;
}

GRect scl_center(GRect r) {
  return scl_center_x(scl_center_y(r));
}

GRect scl_largest_square() {
#if defined(PBL_PLATFORM_CHALK)
  // Side size is diameter * (sqrt(2) / 2)
  const int side = (PS_DISP_W * SQRT_2_OVER_2) / 1000;
  return scl_center(GRect(0, 0, side, side));
#else
  return scl_center(GRect(0, 0, PS_DISP_W, PS_DISP_W));
#endif
}

/////////////////////////////////////////////// Fonts //////////////////////////////////////////////

// Max font sets that can be stored
#define _MAX_FONT_SETS 32

// Macro to pick the pointer based on the build target
#if defined(PBL_PLATFORM_CHALK)
  #define GET_SF(s) ((s).c ? (s).c : (s).o)
#elif defined(PBL_PLATFORM_EMERY)
  #define GET_SF(s) ((s).e ? (s).e : (s).o)
#else
  #define GET_SF(s) ((s).o)
#endif

static GFont s_fonts_ptrs[_MAX_FONT_SETS];

void _scl_set_fonts_impl(int font_id, SF fonts) {
  if (font_id >= _MAX_FONT_SETS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "ps: font id %d exceeds max", font_id);
    return;
  }

  // Pick the correct pointer now and store only that one
  s_fonts_ptrs[font_id] = GET_SF(fonts);
}

GFont scl_get_font(int font_id) {
  if (font_id >= _MAX_FONT_SETS || s_fonts_ptrs[font_id] == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "ps: font %d not set", font_id);
    return NULL; 
  }

  return s_fonts_ptrs[font_id];
}
