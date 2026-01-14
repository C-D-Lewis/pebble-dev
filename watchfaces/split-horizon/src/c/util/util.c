#include "util.h"

void util_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);
  animation_set_curve((Animation*) anim, AnimationCurveEaseInOut);
  animation_schedule((Animation*) anim);
}

TextLayer* util_init_text_layer(GRect location, GColor colour, GColor background, GFont font, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}
