#pragma once

#include <pebble.h>

#include "data.h"
#include "scalable.h"

TextLayer* util_make_text_layer(GRect frame, GFont font);

void util_fmt_time(int timestamp_s, char* buff, int size);

void util_fmt_time_ago(time_t then, char *buff, int size);

int util_hours_until_next_interval();

void util_fmt_time_unit(time_t ts, char *buff, int size);

char* util_get_status_string();

uint32_t util_get_battery_resource_id(int charge_percent);

bool util_is_not_status(int v);

bool util_is_not_charging(int v);

void util_draw_braid(GContext *ctx, GRect rect);

void util_menu_cell_draw(GContext *ctx, Layer *layer, char *title, char *desc);

void util_draw_button_hints(GContext *ctx, bool hints[3]);

#if !defined(PBL_PLATFORM_APLITE)
void util_animate(int duration, int delay, AnimationImplementation *implementation, bool handlers);

int util_anim_percentage(AnimationProgress dist_normalized, int max);

void util_stop_animation();

bool util_is_animating();

void util_stop_animation();
#endif
