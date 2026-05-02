#include "scalable.h"

static GFont
s_font_ost_reg_16,
s_font_ost_reg_22,
s_font_ost_hvy_22,
s_font_ost_hvy_30,
s_font_ost_hvy_40,
s_font_ost_hvy_55;

void scalable_init() {
  s_font_ost_reg_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_REGULAR_16));
  s_font_ost_reg_22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_REGULAR_22));
  s_font_ost_hvy_22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_HEAVY_22));
  s_font_ost_hvy_30 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_HEAVY_30));
  s_font_ost_hvy_40 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_HEAVY_40));
  s_font_ost_hvy_55 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_HEAVY_55));
  
  scl_set_fonts(SFI_Label, {.o = s_font_ost_reg_16, .e = s_font_ost_reg_22});
  scl_set_fonts(SFI_Date, {.o = s_font_ost_hvy_22, .e = s_font_ost_hvy_30});
  scl_set_fonts(SFI_Time, {.o = s_font_ost_hvy_40, .e = s_font_ost_hvy_55});
}
