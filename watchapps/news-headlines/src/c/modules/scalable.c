#include "scalable.h"

static GFont
  s_font_g18,
  s_font_g18b,
  s_font_g24,
  s_font_g24b,
  s_font_g28,
  s_font_g28b;

void scalable_init() {
  // Fonts in use
  s_font_g18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_font_g18b = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_font_g24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_font_g24b = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_font_g28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_font_g28b = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  scl_set_fonts(SFI_Medium, {.o = s_font_g24, .e = s_font_g28});
  scl_set_fonts(SFI_MediumBold, {.o = s_font_g24b, .e = s_font_g28b});

  scl_set_fonts(SFI_StatusLayer, {.o = s_font_g18b, .e = s_font_g24b});
}
