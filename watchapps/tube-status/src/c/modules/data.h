#pragma once

#include <pebble.h>

#include "../config.h"

typedef struct {
  int index;
  char name[32];
  char region[32];
} TransitSystemData;

typedef struct {
  bool configured;
  char name[32];
  uint32_t color;
  bool striped;
} LineConfig;

typedef struct {
  int index;
  char state[32];
  char reason[512];
  StatusSeverity severity;
} LineData;

void data_init();

void data_deinit();

void data_set_transit_system_data(int index, const char *name, const char *region);

char *data_get_transit_system_name(int index);

char *data_get_transit_system_region(int index);

void data_set_line_config(int index, const char *name, uint32_t color, bool striped);

char *data_get_line_name(int index);

LineData *data_get_line(int index);

GColor data_get_line_color(int index);

GColor data_get_line_state_color(int index);

bool data_get_line_color_is_striped(int index);

void data_set_progress(int progress);

int data_get_progress();

void data_set_progress_max(int max);

bool data_get_line_has_reason(int index);

int data_get_progress_max();

int data_get_lines_received();

int data_get_configured_line_count();
