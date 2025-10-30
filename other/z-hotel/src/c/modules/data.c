#include "data.h"

static HotelData s_hotel_data[HotelMax];
static int s_progress = 0;
static int s_progress_max = HotelMax;
static char s_updated_at[32] = "";

void data_init() {
}

void data_deinit() {
}

HotelData* data_get_hotel(int index) {
  return &s_hotel_data[index];
}

void data_set_progress(int progress) {
  s_progress = progress;
}

int data_get_progress() {
  return s_progress;
}

void data_set_progress_max(int max) {
  s_progress_max = max;
}

int data_get_progress_max() {
  return s_progress_max;
}

void data_set_updated_at(const char* updated_at) {
  snprintf(s_updated_at, sizeof(s_updated_at) + 1, "%s\n", updated_at);
}

const char* data_get_updated_at() {
  return &s_updated_at[0];
}
