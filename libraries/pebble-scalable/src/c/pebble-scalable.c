#include "pebble-scalable.h"

#if defined(PBL_PLATFORM_EMERY)
  #define PS_DISPLAY_W 200
  #define PS_DISPLAY_H 228
#elif defined(PBL_PLATFORM_CHALK)
  #define PS_DISPLAY_W 180
  #define PS_DISPLAY_H 180
#else // aplite, basalt, diorite, flint
  #define PS_DISPLAY_W 144
  #define PS_DISPLAY_H 168
#endif

// Max font sets that can be stored
#define PS_MAX_FONT_SETS 32
// Max image ID sets that can be stored
#define PS_MAX_GBITMAP_IDS 32
// Number of distinct screen shapes
#define PS_DISTINCT_SHAPES 3
// Emery distinct screen shape slot
#define PS_DS_EMERY 2
// Chalk distinct screen shape slot
#define PS_DS_CHALK 1
// All other screen shape (aplite, basalt, diorite, flint etc.) slot
#define PS_DS_REGULAR 0
// Half the scale input in thousandths
#define PS_HALF_THOU 500

static GFont *s_fonts_ptrs[PS_MAX_FONT_SETS][PS_DISTINCT_SHAPES];

// static int min_1(int v) {
//   return v < 1 ? 1 : v;
// }

static GFont* get_font_if_set(GFont *ptr) {
  if (ptr == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "pebble-scalable: tried to get font that was not yet set");
  }
  return ptr;
}

static GFont* get_font_from_array(GFont **array) {
#if defined(PBL_PLATFORM_EMERY)
  return array[PS_DS_EMERY];
#elif defined(PBL_PLATFORM_CHALK)
  return array[PS_DS_CHALK];
#else // aplite, basalt, diorite, flint
  return array[PS_DS_REGULAR];
#endif
}

///////////////////////////////////////////// Geometry /////////////////////////////////////////////

int scalable_x(int thou) {
  return ((thou * PS_DISPLAY_W) + PS_HALF_THOU) / 1000;
}

int scalable_y(int thou) {
  return ((thou * PS_DISPLAY_H) + PS_HALF_THOU) / 1000;
}

GRect scalable_grect(int x_perc, int y_perc, int w_perc, int h_perc) {
  return GRect(
    scalable_x(x_perc),
    scalable_y(y_perc),
    scalable_x(w_perc),
    scalable_y(h_perc)
  );
}

GRect scalable_nudge_xy(GRect r, int dx, int dy) {
  r.origin.x += dx;
  r.origin.y += dy;
  return r;
}

GRect scalable_nudge_wh(GRect r, int dw, int dh) {
  r.size.w += dw;
  r.size.h += dh;
  return r;
}

GRect scalable_center_x(GRect r) {
  r.origin.x = (PS_DISPLAY_W - r.size.w) / 2;
  return r;
}

GRect scalable_center_y(GRect r) {
  r.origin.y = (PS_DISPLAY_H - r.size.h) / 2;
  return r;
}

GRect scalable_center(GRect r) {
  return scalable_center_x(scalable_center_y(r));
}

/////////////////////////////////////////////// Fonts //////////////////////////////////////////////

void scalable_set_fonts(int id, GFont *regular, GFont *chalk, GFont *emery) {
  if (id >= PS_MAX_FONT_SETS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "pebble-scalable: font id must be <%d", PS_MAX_FONT_SETS);
    return;
  }

  s_fonts_ptrs[id][PS_DS_REGULAR] = regular;
  s_fonts_ptrs[id][PS_DS_CHALK] = chalk;
  s_fonts_ptrs[id][PS_DS_EMERY] = emery;
}

GFont scalable_get_font(int id) {
  return *(get_font_if_set(get_font_from_array(s_fonts_ptrs[id])));
}
