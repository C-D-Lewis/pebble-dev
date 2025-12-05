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
