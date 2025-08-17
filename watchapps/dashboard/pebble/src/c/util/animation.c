#include "animation.h"

static bool s_locked;

void animation_animate_layer(Layer *layer, GRect start, GRect finish, int duration_ms, AnimationHandlers *handlers) {
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_duration(anim, duration_ms);
  if(handlers) {
    animation_set_handlers(anim, (*handlers), NULL);
  }
  animation_schedule(anim);
}

void animation_animate_delta_y(Layer *layer, int dy, int duration_ms, AnimationHandlers *handlers) {
  GRect start = layer_get_frame(layer);
  GRect finish = GRect(start.origin.x, start.origin.y + dy, start.size.w, start.size.h);
  animation_animate_layer(layer, start, finish, duration_ms, handlers);
}

void animation_lock() {
  s_locked = true;
}

void animation_unlock() {
  s_locked = false;
}

bool animation_is_locked() {
  return s_locked;
}