#include "data.h"

#define NUM_SERVICES 34

static Service s_services[NUM_SERVICES];

static uint8_t *s_data_buffer;

static int parse_two_digit_number(int offset) {
  uint8_t tens = (uint8_t)s_data_buffer[offset];
  tens -= '0';
  uint8_t units = (uint8_t)s_data_buffer[offset + 1];
  units -= '0';
  return (tens * 10) + units;
}

static void unpack_data() {
  int cursor = 0;
  for(int i = 0; i < NUM_SERVICES; i++) {
    s_services[i].depart_hour = parse_two_digit_number(cursor);
    cursor += 3;
    s_services[i].depart_minute = parse_two_digit_number(cursor);
    cursor += 3;
    s_services[i].arrive_hour = parse_two_digit_number(cursor);
    cursor += 3;
    s_services[i].arrive_minute = parse_two_digit_number(cursor);
    cursor += 3;
    s_services[i].platform = parse_two_digit_number(cursor);
    cursor += 3;
  }
}

/************************************ API *************************************/

void data_init() {
  ResHandle handle = resource_get_handle(RESOURCE_ID_DATA);
  size_t res_size = resource_size(handle);
  s_data_buffer = (uint8_t*)malloc(res_size);
  resource_load(handle, s_data_buffer, res_size);
  unpack_data();
}

void data_deinit() {
  if(s_data_buffer) {
    free(s_data_buffer);
  }
}

Service* data_get_service(int index) {
  return &s_services[index];
}

int data_get_num_services() {
  return NUM_SERVICES;
}