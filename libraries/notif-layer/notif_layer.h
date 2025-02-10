#pragma once

#include <pebble.h>

#ifdef PBL_SDK_2
#define STATUS_BAR_LAYER_HEIGHT 16
#endif

#define NOTIF_LAYER_DEFAULT_HEIGHT STATUS_BAR_LAYER_HEIGHT
#define NOTIF_LAYER_MAX_LENGTH 128
#define NOTIF_LAYER_SCROLL_DURATION_MULT 2000

typedef struct {
  Layer *bg_layer;
  TextLayer *content_layer;
  char buffer[NOTIF_LAYER_MAX_LENGTH];
  PropertyAnimation *bg_prop_anim;
  PropertyAnimation *content_prop_anim;
  bool visible;
} NotifLayer;

NotifLayer* notif_layer_create();

void notif_layer_add_to_window(NotifLayer *this, Window *parent);

void notif_layer_destroy(NotifLayer *this);

void notif_layer_show(NotifLayer *this, char *message);

void notif_layer_hide(NotifLayer *this);
