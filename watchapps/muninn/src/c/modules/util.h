#pragma once

#include <pebble.h>

#include "data.h"
#include "scalable.h"

TextLayer* util_make_text_layer(GRect frame, GFont font);

void util_fmt_time(int timestamp_s, char* buff, int size);

void util_fmt_time_ago(time_t then, char *buff, int size);

void util_fmt_time_unit(time_t ts, char *buff, int size);

char* util_get_status_string();

uint32_t util_get_battery_resource_id(int charge_percent);

bool util_is_not_status(int v);

bool util_is_not_charging(int v);

void util_draw_braid(GContext *ctx, GRect rect);

void util_menu_cell_draw(GContext *ctx, Layer *layer, char *title, char *desc);

void util_draw_button_hints(GContext *ctx, bool hints[3]);

void util_draw_skyline(GContext *ctx, bool is_nighttime);

bool util_get_is_night();

uint32_t util_get_mascot_res_id(bool is_enabled, bool is_night);

Layer* util_create_header_layer(char *title, int title_length);

bool util_is_battery_low(int charge_percent);
