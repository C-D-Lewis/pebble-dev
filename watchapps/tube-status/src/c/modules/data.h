#pragma once

#include <pebble.h>

#include "../config.h"

typedef struct {
  int index;  // Unused?
  int type;
  char state[32];
  char reason[256];
} LineData;

void data_init();

void data_deinit();

char* data_get_line_name(int type);

LineData* data_get_line(int index);

GColor data_get_line_color(int type);

GColor data_get_line_state_color(int index);

bool data_get_line_color_is_striped(int type);

void data_set_progress(int progress);

int data_get_progress();

void data_set_progress_max(int max);

bool data_get_line_has_reason(int index);

int data_get_progress_max();
