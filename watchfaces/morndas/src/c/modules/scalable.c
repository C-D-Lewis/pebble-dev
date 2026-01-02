#include "scalable.h"

static GFont s_font_24, s_font_28, s_font_36, s_font_48;

void scalable_init() {
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_24));
  s_font_28 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_28));
  s_font_36 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_36));
  s_font_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURA_48));

  scalable_set_fonts(SFI_MediumBold, &s_font_24, &s_font_28);
  scalable_set_fonts(SFI_Large, &s_font_36, &s_font_48);
}
