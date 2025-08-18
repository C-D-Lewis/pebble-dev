#include "util.h"

static void on_animation_stopped(Animation *anim, bool finished, void *context) {
#ifdef PBL_SDK_2
  property_animation_destroy((PropertyAnimation*) anim);
#endif
}

void util_animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);
  animation_set_curve((Animation*) anim, AnimationCurveEaseInOut);
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler) on_animation_stopped
  };
  animation_set_handlers((Animation*) anim, handlers, NULL);
  animation_schedule((Animation*) anim);
}

TextLayer* util_init_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_load_custom_font(handle));
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}
