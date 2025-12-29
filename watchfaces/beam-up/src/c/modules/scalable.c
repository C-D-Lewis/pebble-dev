#include "scalable.h"

static GFont s_font_24, s_font_34, s_font_48, s_font_65;

void scalable_init() {
  s_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_24));
  s_font_34 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_34));
  s_font_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48));
  s_font_65 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_65));

  scalable_set_fonts(SFI_Small, &s_font_24, &s_font_34);
  scalable_set_fonts(SFI_Large, &s_font_48, &s_font_65);
}
