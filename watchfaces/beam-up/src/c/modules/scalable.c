#include "scalable.h"

static GFont s_font_24, s_font_48, s_font_64;

void scalable_init() {
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24));
  s_font_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48));
  s_font_64 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_64));

  scalable_set_fonts(SFI_Small, &s_font_24, &s_font_24);
  scalable_set_fonts(SFI_Large, &s_font_48, &s_font_64);
}
