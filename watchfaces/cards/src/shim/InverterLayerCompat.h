/**
 * InverterLayer substitute for Basalt
 * Author: Chris Lewis
 *
 * Caveats: 
 * - Must specify colors with inverter_layer_set_colors().
 * - Due to shared framebuffer, there can only be one color set
 */

#ifdef PBL_PLATFORM_BASALT
#pragma once

#include <pebble.h>

typedef struct {
  Layer *layer;
} InverterLayerCompat;

InverterLayerCompat *inverter_layer_compat_create(GRect bounds);
void inverter_layer_compat_set_colors(GColor fg, GColor bg);
void inverter_layer_compat_destroy(InverterLayerCompat *this);
Layer* inverter_layer_compat_get_layer(InverterLayerCompat *this);

#endif
