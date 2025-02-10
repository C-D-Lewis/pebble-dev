/**
 * InverterLayer substitute for Basalt
 * Author: Chris Lewis
 *
 * Caveats: 
 * - Must specify colors with inverter_layer_set_colors().
 * - Due to shared framebuffer, there can only be one color set
 */

#pragma once

#include <pebble.h>

#ifdef PBL_SDK_3

typedef struct {
  Layer *layer;
} InverterLayerCompat;

InverterLayerCompat *inverter_layer_compat_create(GRect bounds);
void inverter_layer_compat_set_colors(GColor fg, GColor bg);
void inverter_layer_compat_destroy(InverterLayerCompat *this);
Layer* inverter_layer_compat_get_layer(InverterLayerCompat *this);

// Compat functions
Layer* inverter_layer_compat_func_get(InverterLayerCompat *layer);
InverterLayerCompat* inverter_layer_compat_func_create(GRect frame);
void inverter_layer_compat_func_destroy(InverterLayerCompat* layer);

#elif PBL_SDK_2

Layer* inverter_layer_compat_func_get(InverterLayer *layer);
InverterLayer* inverter_layer_compat_func_create(GRect frame);
void inverter_layer_compat_func_destroy(InverterLayer* layer);

#endif
