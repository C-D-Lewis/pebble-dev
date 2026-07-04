#include "scalable.h"

void scalable_init() {
  GFont s_font_g24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  GFont s_font_g28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);

  scl_set_fonts(SFI_Medium, {.o = s_font_g24, .e = s_font_g28});
}
