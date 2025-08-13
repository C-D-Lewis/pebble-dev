/**
 * ToastLayer for Pebble
 *
 * Author: Chris Lewis
 *
 * Instructions:
 * 1. Create with message
 * 2. Show when ready
 * 3. Destroy when parent Window is destroyed
 */
#pragma once

#include <pebble.h>

#define TOAST_LAYER_MARGIN        2
#define TOAST_LAYER_ANIM_DURATION 400

typedef struct {
  Window *parent;
  TextLayer *bg_layer;
  TextLayer *content_layer;
  GSize size;
  GRect parent_bounds;
  char *content_buffer;
  int duration;
  bool is_visible;
} ToastLayer;

/**
 * Create a ToastLayer and add it to the parent Window
 */
ToastLayer* toast_layer_create(Window *parent);

/**
 * Destroy a ToastLayer
 */
void toast_layer_destroy(ToastLayer *this);

/**
 * Pop up the ToastLayer
 */
void toast_layer_show(ToastLayer *this, char *message, int duration);

/**
 * Hide the ToastLayer
 */
void toast_layer_hide(ToastLayer *this);

/**
 * Determine if the ToastLayer is still visible
 */
bool toast_layer_is_visible(ToastLayer *this);
