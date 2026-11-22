#include "scalable.h"

static GFont
  s_font_g18,
  s_font_g22,
  s_font_g40;

void scalable_init() {
  s_font_g18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTCHA_GOTHIC_18));
  s_font_g22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTCHA_GOTHIC_22));
  s_font_g40 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTCHA_GOTHIC_40));

  scl_set_fonts(SFI_Small, {.o = s_font_g18});
  scl_set_fonts(SFI_Medium, {.o = s_font_g22});
  scl_set_fonts(SFI_Large, {.o = s_font_g40});
}
