#include "scalable.h"

static GFont s_font_25, s_font_29;

void scalable_init() {
  s_font_25 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BEBAS_NEUE_25));
  s_font_29 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BEBAS_NEUE_29));

  scl_set_fonts(SFI_Date, {.g = s_font_25});
  scl_set_fonts(SFI_DateLarge, {.g = s_font_29});
  scl_set_fonts(SFI_Weather, {.g = s_font_25});
}
