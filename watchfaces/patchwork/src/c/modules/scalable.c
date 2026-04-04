#include "scalable.h"

static GFont s_font_24, s_font_36, s_font_60, s_font_80;

void scalable_init() {
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_24));
  s_font_36 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_36));
  s_font_60 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_60));
  s_font_80 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_80));

  scl_set_fonts(SFI_Small, {.o = s_font_24, .e = s_font_36, .g = s_font_36});
  scl_set_fonts(SFI_Large, {.o = s_font_60, .e = s_font_80, .g = s_font_80});
}
