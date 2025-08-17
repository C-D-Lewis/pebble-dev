#pragma once

#include <pebble.h>

#include "../config.h"
#include "../types.h"

void data_init();
void data_deinit();

void data_set_toggle_state(int type, int state);
int data_get_toggle_state(int type);

void data_set_toggle_type_at_index(int index, int type);
int data_get_toggle_type_at_index(int index);

int data_get_toggle_index_with_type(int type);

void data_set_gsm_name(char *name);
char* data_get_gsm_name();

void data_set_wifi_name(char *name);
char* data_get_wifi_name();

void data_set_gsm_percent(int percent);
int data_get_gsm_percent();

void data_set_battery_percent(int percent);
int data_get_battery_percent();

void data_set_storage_gb_major(int value);
int data_get_storage_gb_major();

void data_set_storage_gb_minor(int value);
int data_get_storage_gb_minor();

void data_set_is_lollipop(bool is_lollipop);
bool data_get_is_lollipop();

void data_set_toggle_order(char *order);
char* data_get_toggle_order();

void data_set_storage_percent(int perc);
int data_get_storage_percent();
