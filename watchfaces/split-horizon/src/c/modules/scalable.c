#include "scalable.h"

static GFont s_font_20, s_font_27, s_font_25, s_font_33, s_font_43, s_font_58;

void scalable_init() {
  // Fonts in use
  s_font_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_20));
  s_font_27 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_27));
  s_font_25 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_25));
  s_font_33 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_33));
  s_font_43 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_43));
  s_font_58 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_58));

  scl_set_fonts(SFI_Small, {.o = s_font_20, .e = s_font_27});
  scl_set_fonts(SFI_Medium, {.o = s_font_25, .e = s_font_33});
  scl_set_fonts(SFI_Large, {.o = s_font_43, .e = s_font_58});
}
