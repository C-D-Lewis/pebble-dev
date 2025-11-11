#include "util.h"

TextLayer* util_make_text_layer(GRect frame) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_font(this, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(this, GColorClear);
  return this;
}
