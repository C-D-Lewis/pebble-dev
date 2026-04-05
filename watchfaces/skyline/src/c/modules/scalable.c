#include "scalable.h"

static GFont s_font_24, s_font_55;

void scalable_init() {
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_24));
  s_font_55 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_55));

  scl_set_fonts(SFI_Regular, {.o = s_font_24});
  scl_set_fonts(SFI_Large, {.o = s_font_55});
}
