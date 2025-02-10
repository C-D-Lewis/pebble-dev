#pragma once

#include <pebble.h>

typedef enum {
  ColorTypeBackground = 0,
  ColorTypeRing,
  ColorTypeDotInactive,
  ColorTypeDotActive,

  ColorTypeCount
} ColorType;

typedef enum {
  FeatureKeyCount = 2,
  FeatureKeyBase = 10,

  FeatureKeyShowMinutes,
  FeatureKeyBTAlert
} FeatureKey;

void data_init();

void data_deinit();

void data_set_color(ColorType type, GColor color);

GColor data_get_color(ColorType type);

void data_set_feature(FeatureKey key, bool b);

bool data_get_feature(FeatureKey key);
