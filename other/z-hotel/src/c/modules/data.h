#pragma once

#include <pebble.h>

typedef struct {
  int index;
  char name[32];
  char summary[256];
} HotelData;

#define HotelMax 11

void data_init();

void data_deinit();

HotelData* data_get_hotel(int index);

void data_set_progress(int progress);

int data_get_progress();

void data_set_progress_max(int max);

int data_get_progress_max();

void data_set_updated_at(const char* updated_at);

const char* data_get_updated_at();
