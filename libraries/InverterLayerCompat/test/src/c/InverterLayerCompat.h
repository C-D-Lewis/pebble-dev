/**
 * InverterLayer substitute
 * Author: Chris Lewis
 */

#pragma once

#include <pebble.h>

typedef struct {
  GColor fg_color;
  GColor bg_color;
} InverterLayerCompatInfo;

typedef struct {
  Layer *layer;
} InverterLayerCompat;

InverterLayerCompat *inverter_layer_compat_create(GRect bounds, GColor fg, GColor bg);
void inverter_layer_compat_destroy(InverterLayerCompat *this);
Layer* inverter_layer_compat_get_layer(InverterLayerCompat *this);
