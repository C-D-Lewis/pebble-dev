#include "ToastLayer.h"

ToastLayer* toast_layer_create(Window *parent) {
  ToastLayer *this = malloc(sizeof(ToastLayer));
  this->parent = parent;
  this->parent_bounds = layer_get_bounds(window_get_root_layer(parent));

  return this;
}

void toast_layer_destroy(ToastLayer *this) {
  text_layer_destroy(this->bg_layer);
  text_layer_destroy(this->content_layer);

  free(this);
}

static void anim_stopped(struct Animation *animation, bool finished, void *context) {
#ifdef PBL_PLATFORM_APLITE
  property_animation_destroy((PropertyAnimation*)animation);
#endif
}

static void animate_layer(Layer *layer, GRect start, GRect finish, int duration) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*)anim, duration);
  animation_set_curve((Animation*)anim, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)anim, (AnimationHandlers) {
    .stopped = anim_stopped
  }, NULL);
  animation_schedule((Animation*)anim);
}

void toast_layer_hide(ToastLayer *this) {
  if(this->is_visible) {
    Layer *text_layer = text_layer_get_layer(this->bg_layer);

    // Background
    GRect start = layer_get_frame(text_layer);
    GRect finish = GRect(0, this->parent_bounds.size.h, 144, this->size.h + TOAST_LAYER_MARGIN);
    animate_layer(text_layer, start, finish, TOAST_LAYER_ANIM_DURATION);

    // Content
    start = layer_get_frame(text_layer_get_layer(this->content_layer));
    finish = GRect(TOAST_LAYER_MARGIN, this->parent_bounds.size.h + TOAST_LAYER_MARGIN, 144 - (2 * TOAST_LAYER_MARGIN), 0);
    animate_layer(text_layer_get_layer(this->content_layer), start, finish, TOAST_LAYER_ANIM_DURATION);
  }
}

static void timer_callback(void *context) {
  ToastLayer *this = (ToastLayer*)context;

  toast_layer_hide(this);
  this->is_visible = false;
}

void toast_layer_show(ToastLayer *this, char *message, int duration) {
  if(!this->is_visible) {
    this->is_visible = true;

    // Set up
    this->duration = duration;

    // Allocate buffer
    int length = strlen(message) + 1;
    this->content_buffer = malloc(length * sizeof(char));
    snprintf(this->content_buffer, length * sizeof(char), "%s", message);

    // Create large, scale down
    this->content_layer = text_layer_create(GRect(TOAST_LAYER_MARGIN, 168 + TOAST_LAYER_MARGIN, 144 - (2 * TOAST_LAYER_MARGIN), 168));
    text_layer_set_text(this->content_layer, this->content_buffer);
    text_layer_set_background_color(this->content_layer, GColorWhite);

    // Create offscreen according to size used by TextLayer
    this->size = text_layer_get_content_size(this->content_layer);
    this->size.h += 10; // Better wrapping
    this->bg_layer = text_layer_create(GRect(0, this->parent_bounds.size.h, 144, this->size.h + TOAST_LAYER_MARGIN));
    text_layer_set_background_color(this->bg_layer, GColorBlack);

    // Add BG first
    layer_add_child(window_get_root_layer(this->parent), text_layer_get_layer(this->bg_layer));
    layer_add_child(window_get_root_layer(this->parent), text_layer_get_layer(this->content_layer));

    // Background
    GRect start = GRect(0, this->parent_bounds.size.h, 144, 0);
    GRect finish = GRect(0, this->parent_bounds.size.h - this->size.h - TOAST_LAYER_MARGIN, 144, this->size.h + (2 * TOAST_LAYER_MARGIN));
    animate_layer(text_layer_get_layer(this->bg_layer), start, finish, TOAST_LAYER_ANIM_DURATION);

    // Content
    start = GRect(TOAST_LAYER_MARGIN, this->parent_bounds.size.h, 144, 0);
    finish = GRect(TOAST_LAYER_MARGIN, this->parent_bounds.size.h - this->size.h + TOAST_LAYER_MARGIN, 144 - (2 * TOAST_LAYER_MARGIN), this->size.h);
    text_layer_set_text(this->content_layer, this->content_buffer);
    animate_layer(text_layer_get_layer(this->content_layer), start, finish, TOAST_LAYER_ANIM_DURATION);

    // Auto hide
    app_timer_register(this->duration, timer_callback, this);
  }
}

bool toast_layer_is_visible(ToastLayer *this) {
  return this->is_visible;
}
