#pragma once

#include <pebble.h>

// Platforms:
// http://www.brtimes.com/#!board?stn=LST&filt=WTM&date=20161003

typedef struct {
  int arrive_hour;
  int arrive_minute;
  int depart_hour;
  int depart_minute;
  int platform;
} Service;

void data_init();

void data_deinit();

Service* data_get_service(int index);

int data_get_num_services();
