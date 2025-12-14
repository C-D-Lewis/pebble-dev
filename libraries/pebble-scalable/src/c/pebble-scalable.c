#include "pebble-scalable.h"

#if defined(PBL_PLATFORM_EMERY)
  #define PS_DISPLAY_W 200
  #define PS_DISPLAY_H 228
#else // aplite, basalt, diorite, flint
  #define PS_DISPLAY_W 144
  #define PS_DISPLAY_H 168
#endif

// Max font sets that can be stored
#define PS_MAX_FONT_SETS 32
// Max image ID sets that can be stored
#define PS_MAX_GBITMAP_IDS 32
// Number of distinct screen shapes
#define PS_DISTINCT_SHAPES 2
// All other screen shape (aplite, basalt, diorite, flint etc.) slot
#define PS_DS_REGULAR 0
// Emery distinct screen shape slot
#define PS_DS_EMERY 1
// Half the scale input in thousandths
#define PS_HALF_RANGE 500

static GFont *s_fonts_ptrs[PS_MAX_FONT_SETS][PS_DISTINCT_SHAPES];

static GFont* get_font_if_set(GFont *ptr) {
  if (ptr == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "pebble-scalable: tried to get font that was not yet set");
  }
  return ptr;
}

static GFont* get_font_from_array(GFont **array) {
#if defined(PBL_PLATFORM_EMERY)
  return array[PS_DS_EMERY];
#else // aplite, basalt, diorite, flint
  return array[PS_DS_REGULAR];
#endif
}

///////////////////////////////////////////// Geometry /////////////////////////////////////////////

int scalable_x(int t_perc) {
  return ((t_perc * PS_DISPLAY_W) + PS_HALF_RANGE) / 1000;
}

int scalable_y(int t_perc) {
  return ((t_perc * PS_DISPLAY_H) + PS_HALF_RANGE) / 1000;
}

int scalable_x_pp(int regular, int emery) {
#if defined(PBL_PLATFORM_EMERY)
  return scalable_x(emery);
#else // aplite, basalt, diorite, flint
  return scalable_x(regular);
#endif
}

int scalable_y_pp(int regular, int emery) {
#if defined(PBL_PLATFORM_EMERY)
  return scalable_y(emery);
#else // aplite, basalt, diorite, flint
  return scalable_y(regular);
#endif
}

GRect scalable_grect(int x_t_perc, int y_t_perc, int w_t_perc, int h_t_perc) {
  return GRect(
    scalable_x(x_t_perc),
    scalable_y(y_t_perc),
    scalable_x(w_t_perc),
    scalable_y(h_t_perc)
  );
}

GRect scalable_grect_pp(GRect regular, GRect emery) {
#if defined(PBL_PLATFORM_EMERY)
  const GRect val = emery;
#else // aplite, basalt, diorite, flint
  const GRect val = regular;
#endif
  return GRect(
    scalable_x(val.origin.x),
    scalable_y(val.origin.y),
    scalable_x(val.size.w),
    scalable_y(val.size.h)
  );
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

void scalable_set_fonts(int id, GFont *regular, GFont *emery) {
  if (id >= PS_MAX_FONT_SETS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "pebble-scalable: font id must be <%d", PS_MAX_FONT_SETS);
    return;
  }

  s_fonts_ptrs[id][PS_DS_REGULAR] = regular;
  s_fonts_ptrs[id][PS_DS_EMERY] = emery;
}

GFont scalable_get_font(int id) {
  return *(get_font_if_set(get_font_from_array(s_fonts_ptrs[id])));
}
