#include "scalable.h"

static GFont s_font_g24;

void scalable_init() {
  s_font_g24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);

  scl_set_fonts(SFI_Medium, {.o = s_font_g24, .e = s_font_g24});
}
