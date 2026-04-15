#include "scalable.h"

static GFont s_font_14, s_font_19, s_font_25, s_font_30, s_font_38, s_font_55;

void scalable_init() {
  s_font_14 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_14));
  s_font_19 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_19));
  s_font_25 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_25));
  s_font_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_30));
  s_font_38 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_38));
  s_font_55 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OXANIUM_55));

  scl_set_fonts(SFI_Weather, {.o = s_font_14, .c = s_font_19, .e = s_font_19, .g = s_font_25});
  scl_set_fonts(SFI_Date, {.o = s_font_14, .c = s_font_19, .e = s_font_19, .g = s_font_25});
  scl_set_fonts(SFI_Time, {.o = s_font_30, .c = s_font_38, .e = s_font_38, .g = s_font_55});
}
