#include "scalable.h"

static GFont s_font_g18, s_font_g24, s_font_g24_b, s_font_g28, s_font_g28_b;

void scalable_init() {
  // Fonts in use
  s_font_g18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_font_g24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_font_g24_b = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_font_g28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_font_g28_b = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  scalable_set_fonts(SFI_Small, &s_font_g18, NULL, &s_font_g24);
  scalable_set_fonts(SFI_Medium, &s_font_g24, NULL, &s_font_g28);
  scalable_set_fonts(SFI_MediumBold, &s_font_g24_b, NULL, &s_font_g24_b);
  scalable_set_fonts(SFI_Large, &s_font_g28, NULL, &s_font_g28);
  scalable_set_fonts(SFI_LargeBold, &s_font_g28_b, NULL, &s_font_g28_b);
}
