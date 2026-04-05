#include "scalable.h"

static GFont s_font_18, s_font_24, s_font_38, s_font_55;

void scalable_init() {
  s_font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_18));
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_24));
  s_font_38 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_38));
  s_font_55 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_55));

  scl_set_fonts(SFI_Regular, {.c = s_font_18, .e = s_font_18, .g = s_font_24});
  scl_set_fonts(SFI_Large, {.c = s_font_38, .e = s_font_38, .g = s_font_55});
}
