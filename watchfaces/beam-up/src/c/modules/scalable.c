#include "scalable.h"

static GFont s_font_24, s_font_34, s_font_48, s_font_65;

void scalable_init() {
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24));
  s_font_34 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_34));
  s_font_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48));
  s_font_65 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_65));

  scl_set_fonts(SFI_Small, {.o = s_font_24, .e = s_font_34, .g = s_font_34});
  scl_set_fonts(SFI_Large, {.o = s_font_48, .e = s_font_65, .g = s_font_65});
}
