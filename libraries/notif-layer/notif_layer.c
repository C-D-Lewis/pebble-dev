#include "notif_layer.h"

static void update_proc(Layer *layer, GContext *ctx) {
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

NotifLayer* notif_layer_create() {
  NotifLayer *this = (NotifLayer*)malloc(sizeof(NotifLayer));
  this->bg_layer = layer_create(GRect(0, -NOTIF_LAYER_DEFAULT_HEIGHT, 144, NOTIF_LAYER_DEFAULT_HEIGHT));
  layer_set_update_proc(this->bg_layer, update_proc);
  this->content_layer = text_layer_create(GRect(0, -NOTIF_LAYER_DEFAULT_HEIGHT, 144, NOTIF_LAYER_DEFAULT_HEIGHT));
  text_layer_set_text_color(this->content_layer, GColorWhite);
  text_layer_set_background_color(this->content_layer, GColorClear);
  text_layer_set_overflow_mode(this->content_layer, GTextOverflowModeWordWrap);
  this->visible = false;

  return this;
}

void notif_layer_add_to_window(NotifLayer *this, Window *parent) {
  Layer *parent_layer = window_get_root_layer(parent);
  layer_add_child(parent_layer, this->bg_layer);
  layer_add_child(parent_layer, text_layer_get_layer(this->content_layer));
}

static void destroy_animations(NotifLayer *this) {
  if(this->content_prop_anim) {
    animation_unschedule((Animation*)this->content_prop_anim);
#ifdef PBL_SDK_2
    property_animation_destroy(this->content_prop_anim);
    this->content_prop_anim = NULL;
#endif
  }
  if(this->bg_prop_anim) {
    animation_unschedule((Animation*)this->bg_prop_anim);
#ifdef PBL_SDK_2
    property_animation_destroy(this->bg_prop_anim);
    this->bg_prop_anim = NULL;
#endif
  }
}

void notif_layer_destroy(NotifLayer *this) {
  destroy_animations(this);

  layer_destroy(this->bg_layer);
  text_layer_destroy(this->content_layer);
  free(this);
}

static void scroll_stopped_handler(Animation *animation, bool finished, void *context) {
  NotifLayer *this = (NotifLayer*)context;

  // Hide
  notif_layer_hide(this);
}

static void show_stopped_handler(Animation *animation, bool finished, void *context) {
  NotifLayer *this = (NotifLayer*)context;

  Layer *content_layer = text_layer_get_layer(this->content_layer);

  // Shrink wrap
  text_layer_set_size(this->content_layer, GSize(1000, NOTIF_LAYER_DEFAULT_HEIGHT));
  GSize size = text_layer_get_content_size(this->content_layer);
  size.w += 10;
  layer_set_frame(content_layer, GRect(0, 0, size.w, size.h));

  // Marquee text
  GRect start = GRect(0, 0, size.w, size.h);
  GRect finish = GRect((size.w > 144) ? (-size.w + 144) : 0, 0, size.w, size.h);
  this->content_prop_anim = property_animation_create_layer_frame(content_layer, &start, &finish);
  Animation *content_anim = property_animation_get_animation(this->content_prop_anim);
  animation_set_duration(content_anim, NOTIF_LAYER_SCROLL_DURATION_MULT + (NOTIF_LAYER_SCROLL_DURATION_MULT * (size.w / 144)));
  animation_set_delay(content_anim, NOTIF_LAYER_SCROLL_DURATION_MULT);
  animation_set_curve(content_anim, AnimationCurveLinear);
  animation_set_handlers(content_anim, (AnimationHandlers) {
    .stopped = scroll_stopped_handler
  }, this);
  animation_schedule(content_anim);
}

void notif_layer_show(NotifLayer *this, char *message) {
  if(!this->visible) {
    this->visible = true;
    
    Layer *bg_layer = this->bg_layer;
    Layer *content_layer = text_layer_get_layer(this->content_layer);

    snprintf(this->buffer, NOTIF_LAYER_MAX_LENGTH, "%s", message);
    text_layer_set_text(this->content_layer, this->buffer);

    // Reset
    destroy_animations(this);
    layer_set_frame(content_layer, GRect(0, -NOTIF_LAYER_DEFAULT_HEIGHT, 144, NOTIF_LAYER_DEFAULT_HEIGHT));
    layer_set_frame(bg_layer, GRect(0, -NOTIF_LAYER_DEFAULT_HEIGHT, 144, NOTIF_LAYER_DEFAULT_HEIGHT));

    // Show Animation
    GRect start = layer_get_frame(bg_layer);
    GRect finish = GRect(0, 0, 144, NOTIF_LAYER_DEFAULT_HEIGHT);
    this->bg_prop_anim = property_animation_create_layer_frame(bg_layer, &start, &finish);
    Animation *bg_anim = property_animation_get_animation(this->bg_prop_anim);
    animation_set_handlers(bg_anim, (AnimationHandlers) {
      .stopped = show_stopped_handler
    }, this);
    animation_schedule(bg_anim);

    start = layer_get_frame(content_layer);
    this->content_prop_anim = property_animation_create_layer_frame(content_layer, &start, &finish);
    Animation *content_anim = property_animation_get_animation(this->content_prop_anim);
    animation_schedule(content_anim);
  }
}

static void hide_stopped_handler(Animation *animation, bool finished, void *context) {
  NotifLayer *this = (NotifLayer*)context;
  destroy_animations(this);

  this->visible = false;
}

void notif_layer_hide(NotifLayer *this) {
  Layer *bg_layer = this->bg_layer;
  Layer *content_layer = text_layer_get_layer(this->content_layer);

  GRect start = layer_get_frame(bg_layer);
  GRect finish = GRect(0, -NOTIF_LAYER_DEFAULT_HEIGHT, start.size.w, NOTIF_LAYER_DEFAULT_HEIGHT);
  this->bg_prop_anim = property_animation_create_layer_frame(bg_layer, &start, &finish);
  Animation *bg_anim = property_animation_get_animation(this->bg_prop_anim);
  animation_set_delay(bg_anim, NOTIF_LAYER_SCROLL_DURATION_MULT);
  animation_set_handlers(bg_anim, (AnimationHandlers) {
    .stopped = hide_stopped_handler
  }, this);
  animation_schedule(bg_anim);

  start = layer_get_frame(content_layer);
  finish = GRect(start.origin.x, -NOTIF_LAYER_DEFAULT_HEIGHT, start.size.w, NOTIF_LAYER_DEFAULT_HEIGHT);
  this->content_prop_anim = property_animation_create_layer_frame(content_layer, &start, &finish);
  Animation *content_anim = property_animation_get_animation(this->content_prop_anim);
  animation_set_delay(content_anim, NOTIF_LAYER_SCROLL_DURATION_MULT);
  animation_schedule(content_anim);
}
