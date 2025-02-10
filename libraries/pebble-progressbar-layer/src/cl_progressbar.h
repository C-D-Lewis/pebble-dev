/*
 * ProgressBarLayer code
 * src: https://github.com/C-D-Lewis/pebble-progressbar-layer
 */

#include <pebble.h>

#ifndef CL_PROGRESSBAR_H
#define CL_PROGRESSBAR_H

typedef struct {
  Layer *layer;
  //Progress value is embedded in Layer
} ProgressBarLayer;

ProgressBarLayer* progressbar_layer_create(GRect bounds);
void progressbar_layer_destroy();
void progressbar_layer_set_progress(ProgressBarLayer *this, int new_progress);
int progressbar_layer_get_progress(ProgressBarLayer *this);
Layer* progressbar_layer_get_layer(ProgressBarLayer *this);

#endif