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

#define PS_DISTINCT_SHAPES 3
#define PS_DS_EMERY 2
#define PS_DS_CHALK 1
#define PS_DS_REGULAR 0

static GFont *s_fonts_small[PS_DISTINCT_SHAPES];
static GFont *s_fonts_medium[PS_DISTINCT_SHAPES];
static GFont *s_fonts_large[PS_DISTINCT_SHAPES];

static int min_1(int v) {
  return v < 1 ? 1 : v;
}

int scalable_x(int perc) {
  return min_1((perc * PS_DISPLAY_W) / 100);
}

int scalable_y(int perc) {
  return min_1((perc * PS_DISPLAY_H) / 100);
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

void scalable_set_medium_fonts(GFont *regular, GFont *chalk, GFont *emery) {
  s_fonts_medium[PS_DS_REGULAR] = regular;
  s_fonts_medium[PS_DS_CHALK] = chalk;
  s_fonts_medium[PS_DS_EMERY] = emery;
}

static GFont* get_font_if_set(GFont *ptr) {
  if (ptr == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Tried to get pebble-scalable font that was not yet set");
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

GFont scalable_get_small_font() {
  return *(get_font_if_set(get_font_from_array(s_fonts_small)));
}

GFont scalable_get_medium_font() {
  return *(get_font_if_set(get_font_from_array(s_fonts_medium)));
}

GFont scalable_get_large_font() {
  return *(get_font_if_set(get_font_from_array(s_fonts_large)));
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
