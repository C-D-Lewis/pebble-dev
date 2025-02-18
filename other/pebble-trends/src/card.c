#include "card.h"

Card* card_create() {
  // Allocate
  Card *this = malloc(sizeof(Card));
  this->title_layer = text_layer_create(GRect(5, 0, 0, 0));
  this->timestamp_layer = text_layer_create(GRect(5, 0, 0, 0));
  this->title_buff = malloc((CARD_BUFFER_SIZE * sizeof(char)) + 1);
  this->timestamp_buff = malloc((CARD_BUFFER_SIZE * sizeof(char)) + 1);

  // Setup
  text_layer_set_font(this->title_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_BOLD_24)));
  text_layer_set_text_color(this->title_layer, GColorWhite);
  text_layer_set_background_color(this->title_layer, GColorClear);

  text_layer_set_font(this->timestamp_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_BOLD_20)));

  return this;
}

void card_destroy(Card *this) {
  text_layer_destroy(this->title_layer);
  text_layer_destroy(this->timestamp_layer);

  free(this->title_buff);
  free(this->timestamp_buff);

  free(this);
}

static void recalculate_layout(Card *this) {
  // Force larger for reflow?
  layer_set_frame(text_layer_get_layer(this->title_layer), GRect(5, 0, 144, 168));
  layer_set_frame(text_layer_get_layer(this->timestamp_layer), GRect(5, 0, 144, 168));

  int t_height = text_layer_get_content_size(this->title_layer).h;
  int d_height = text_layer_get_content_size(this->timestamp_layer).h;
  layer_set_frame(text_layer_get_layer(this->title_layer), GRect(5, 0, 144, t_height + 10));

  // Set description offset
  GRect desc_frame = GRect(5, 168 - d_height - 21, 144, d_height + 30);
  layer_set_frame(text_layer_get_layer(this->timestamp_layer), desc_frame);
}

void card_add_to_window(Card *this, Window *parent) {
  // Add children
  layer_add_child(window_get_root_layer(parent), text_layer_get_layer(this->title_layer));
  layer_add_child(window_get_root_layer(parent), text_layer_get_layer(this->timestamp_layer));

  // Calculate height
  recalculate_layout(this);
}

void card_set_title(Card *this, char *title) {
  snprintf(this->title_buff, (strlen(title) * sizeof(char)) + 1, "%s", title);
  text_layer_set_text(this->title_layer, this->title_buff);

  recalculate_layout(this);
}

void card_set_description(Card *this, char *desc) {
  snprintf(this->timestamp_buff, (strlen(desc) * sizeof(char)) + 1, "%s", desc);
  text_layer_set_text(this->timestamp_layer, this->timestamp_buff);

  recalculate_layout(this);
}

static void anim_stopped(struct Animation *animation, bool finished, void *context) {
  property_animation_destroy((PropertyAnimation*)animation);
}

static void animate_layer(Layer *layer, GRect start, GRect finish) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*)anim, 300);
  animation_set_curve((Animation*)anim, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)anim, (AnimationHandlers) {
    .stopped = anim_stopped
  }, NULL);
  animation_schedule((Animation*)anim);
}

void card_hide(Card *this) {
  animate_layer(text_layer_get_layer(this->title_layer), layer_get_frame(text_layer_get_layer(this->title_layer)), GRect(5, 168, 144, 168));
  animate_layer(text_layer_get_layer(this->timestamp_layer), layer_get_frame(text_layer_get_layer(this->timestamp_layer)), GRect(5, 168 + (168 - 16 - 24), 144, 30));
}

void card_show(Card *this) {
  animate_layer(text_layer_get_layer(this->title_layer), layer_get_frame(text_layer_get_layer(this->title_layer)), GRect(5, 0, 144, 168));
  animate_layer(text_layer_get_layer(this->timestamp_layer), layer_get_frame(text_layer_get_layer(this->timestamp_layer)), GRect(5, 168 - 16 - 24, 144, 30));
}
