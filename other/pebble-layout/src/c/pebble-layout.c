#include "pebble-layout.h"

#if defined(PBL_PLATFORM_EMERY)
  #define PL_DISPLAY_W 200
  #define PL_DISPLAY_H 228
#elif defined(PBL_PLATFORM_CHALK)
  #define PL_DISPLAY_W 180
  #define PL_DISPLAY_H 180
#else // aplite, basalt, diorite, flint
  #define PL_DISPLAY_W 144
  #define PL_DISPLAY_H 168
#endif

#define PL_DISTINCT_SHAPES 3
#define PL_DS_EMERY 2
#define PL_DS_CHALK 1
#define PL_DS_REGULAR 0

static GFont *s_fonts_small[PL_DISTINCT_SHAPES];
static GFont *s_fonts_medium[PL_DISTINCT_SHAPES];
static GFont *s_fonts_large[PL_DISTINCT_SHAPES];

static int min_1(int v) {
  return v < 1 ? 1 : v;
}

int pl_x(int perc) {
  return min_1((perc * PL_DISPLAY_W) / 100);
}

int pl_y(int perc) {
  return min_1((perc * PL_DISPLAY_H) / 100);
}

GRect pl_grect(int x_perc, int y_perc, int w_perc, int h_perc) {
  return GRect(pl_x(x_perc), pl_y(y_perc), pl_x(w_perc), pl_y(h_perc));
}

GRect pl_nudge_xy(GRect r, int dx, int dy) {
  r.origin.x += dx;
  r.origin.y += dy;
  return r;
}

GRect pl_nudge_wh(GRect r, int dw, int dh) {
  r.size.w += dw;
  r.size.h += dh;
  return r;
}

void pl_set_medium_fonts(GFont *regular, GFont *chalk, GFont *emery) {
  s_fonts_medium[PL_DS_REGULAR] = regular;
  s_fonts_medium[PL_DS_CHALK] = chalk;
  s_fonts_medium[PL_DS_EMERY] = emery;
}

static GFont* get_font_if_set(GFont *ptr) {
  if (ptr == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Tried to get pebble-layout font that was not yet set");
  }
  return ptr;
}

static GFont* get_font_from_array(GFont **array) {
#if defined(PBL_PLATFORM_EMERY)
  return array[PL_DS_EMERY];
#elif defined(PBL_PLATFORM_CHALK)
  return array[PL_DS_CHALK];
#else // aplite, basalt, diorite, flint
  return array[PL_DS_REGULAR];
#endif
}

GFont pl_get_small_font() {
  return *(get_font_if_set(get_font_from_array(s_fonts_small)));
}

GFont pl_get_medium_font() {
  return *(get_font_if_set(get_font_from_array(s_fonts_medium)));
}

GFont pl_get_large_font() {
  return *(get_font_if_set(get_font_from_array(s_fonts_large)));
}
