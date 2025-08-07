#pragma once

#include <pebble.h>
#include "include/cl_util.h"

#define CARD_SPAWN_LOCATION GRect(0, 168, 144, 109)
#define CARD_RESTING_LOCATION GRect(0, 29, 144, 109)
#define CARD_DESPAWN_LOCATION GRect(0, -100, 144, 109)
#define BAT_ICN_W 9
#define BAT_ICN_H 20

typedef struct {
  // Layout
  GBitmap 
    *background_bitmap,
    *battery_bitmap,
    *bt_bitmap,
    *weather_bitmap;
  BitmapLayer 
    *background_layer,
    *battery_bg_layer,
    *bt_layer,
    *weather_layer; //Assign and free when weather changes
  Layer *battery_bar_layer;

  // Text
  TextLayer 
    *time_layer,
    *date_layer,
    *conditions_layer,
    *location_layer;

  // Storage
  char 
    *time_buff,
    *date_buff;

} Card;

Card* card_add(Window *window, char *location_buff_ptr, char* conditions_buff_ptr);
void card_destroy(Card *this);
void card_spawn(Card *this);
void card_despawn(Card *this);
