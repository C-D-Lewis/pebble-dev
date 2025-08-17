#pragma once

#include <pebble.h>

void animation_animate_layer(Layer *layer, GRect start, GRect finish, int duration, AnimationHandlers *handlers);

void animation_animate_delta_y(Layer *layer, int dy, int duration, AnimationHandlers *handlers);

void animation_lock();

void animation_unlock();

bool animation_is_locked();
