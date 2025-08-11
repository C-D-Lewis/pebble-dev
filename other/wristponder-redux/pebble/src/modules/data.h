#pragma once

#include <pebble.h>

#include "comm.h"

#include "../types.h"

#include "../common/responses_window.h"

#define DATA_MAX_STREAMED_CONTACTS 5
#define DATA_MAX_RESPONSES         30
#define DATA_MAX_NAME_LENGTH       64
#define DATA_MAX_RESPONSES         30
#define DATA_MAX_RESPONSE_LENGTH   102
#define DATA_NUM_CONTACTS          (2 * DATA_MAX_STREAMED_CONTACTS) + 2
#define DATA_UNKNOWN_STRING        "Unknown"

void data_init();
void data_deinit();

void data_set_contact_name(ContactType type, int index, char *name);
char* data_get_contact_name(ContactType type, int index);

void data_set_response(int index, char *response);
char* data_get_response(int index);

bool data_get_contact_valid(ContactType type, int index);

void data_set_total_responses(int total_responses);
int data_get_total_responses();

void data_set_obtained_responses(int obtained_responses);
int data_get_obtained_responses();

bool data_get_response_valid(int index);
