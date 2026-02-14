#include "scalable.h"

static GFont
  s_font_g18,
  s_font_g24,
  s_font_g24_b,
  s_font_g28,
  s_font_g28_b;
#if defined(PBL_PLATFORM_EMERY)
static GFont s_font_gg30;
#endif

void scalable_init() {
  // Fonts in use
  s_font_g18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_font_g24 = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  s_font_g24_b = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_font_g28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  s_font_g28_b = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
#if defined(PBL_PLATFORM_EMERY)
  s_font_gg30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTCHA_GOTHIC_30));
#endif

  scl_set_fonts(SFI_Small, {.o = s_font_g18, .e = s_font_g24});
  scl_set_fonts(SFI_Medium, {.o = s_font_g24, .e = s_font_g28});
  scl_set_fonts(SFI_MediumBold, {.o = s_font_g24_b, .e = s_font_g28_b});
  scl_set_fonts(SFI_Large, {.o = s_font_g28, .e = s_font_g28});
#if defined(PBL_PLATFORM_EMERY)
  scl_set_fonts(SFI_LargeBold, {.o = s_font_g28_b, .e = s_font_gg30});
#else
  // On Aplite, largest font is never used, save space
  scl_set_fonts(SFI_LargeBold, {.o = s_font_g28_b, .e = s_font_g28_b});
#endif
}
