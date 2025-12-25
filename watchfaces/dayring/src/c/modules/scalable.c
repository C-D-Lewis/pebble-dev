#include "scalable.h"

static GFont s_font_48, s_font_64;

void scalable_init() {
  s_font_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TUNGSTEN_48));
  s_font_64 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TUNGSTEN_64));

  scalable_set_fonts(SFI_Regular, &s_font_48, &s_font_64);
}
