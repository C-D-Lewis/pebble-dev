#include "scalable.h"

static GFont s_stm_24, s_stm_36;

void scalable_init() {
  // Fonts in use
  s_stm_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SHARE_24));
  s_stm_36 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SHARE_36));

  scl_set_fonts(SFI_Medium, {.o = s_stm_24, .e = s_stm_36});
}
