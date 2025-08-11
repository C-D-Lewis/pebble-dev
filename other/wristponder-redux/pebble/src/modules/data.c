#include "data.h"

// ~1.2k
static char s_sms_name[DATA_MAX_NAME_LENGTH];
static char s_call_name[DATA_MAX_NAME_LENGTH];
static char s_incoming_names[DATA_MAX_STREAMED_CONTACTS][DATA_MAX_NAME_LENGTH];
static char s_favorite_names[DATA_MAX_STREAMED_CONTACTS][DATA_MAX_NAME_LENGTH];

// More
static char s_responses[DATA_MAX_RESPONSES][DATA_MAX_RESPONSE_LENGTH];
static bool s_contact_valids[DATA_NUM_CONTACTS];
static bool s_response_valids[DATA_MAX_RESPONSES];

static int s_total_responses, s_obtained_responses;

void data_init() {
  // Not yet known
  snprintf(s_sms_name, DATA_MAX_NAME_LENGTH, DATA_UNKNOWN_STRING);
  s_contact_valids[0] = false;
  snprintf(s_call_name, DATA_MAX_NAME_LENGTH, DATA_UNKNOWN_STRING);
  s_contact_valids[1] = false;
  for(int i = 0; i < DATA_MAX_STREAMED_CONTACTS; i++) {
    snprintf(s_incoming_names[i], DATA_MAX_NAME_LENGTH, DATA_UNKNOWN_STRING);
    s_contact_valids[i + 2] = false;
    snprintf(s_favorite_names[i], DATA_MAX_NAME_LENGTH, DATA_UNKNOWN_STRING);
    s_contact_valids[i + 7] = false;
  }
  for(int i = 0; i < DATA_MAX_RESPONSES; i++) {
    snprintf(s_responses[i], DATA_MAX_RESPONSE_LENGTH, DATA_UNKNOWN_STRING);
    s_response_valids[i] = false;
  }
}

void data_deinit() { }

void data_set_contact_name(ContactType type, int index, char *name) {
  switch(type) {
    case ContactTypeSMS:
      strncpy(s_sms_name, name, DATA_MAX_NAME_LENGTH);
      s_contact_valids[0] = (strcmp(name, DATA_UNKNOWN_STRING) != 0);
      break;
    case ContactTypeCall:
      strncpy(s_call_name, name, DATA_MAX_NAME_LENGTH);
      s_contact_valids[1] = (strcmp(name, DATA_UNKNOWN_STRING) != 0);
      break;
    case ContactTypeIncoming:
      strncpy(s_incoming_names[index], name, DATA_MAX_NAME_LENGTH);
      s_contact_valids[index + 2] = (strcmp(name, DATA_UNKNOWN_STRING) != 0);
      break;
    case ContactTypeFavorite:
      strncpy(s_favorite_names[index], name, DATA_MAX_NAME_LENGTH);
      s_contact_valids[index + 7] = (strcmp(name, DATA_UNKNOWN_STRING) != 0);
      break;
    default: break;
  }
}

char* data_get_contact_name(ContactType type, int index) {
  switch(type) {
    case ContactTypeSMS:      return s_sms_name;
    case ContactTypeCall:     return s_call_name;
    case ContactTypeIncoming: return s_incoming_names[index];
    case ContactTypeFavorite: return s_favorite_names[index];
    default: break;
  }
  return "NONE";
}

void data_set_response(int index, char *response) {
  snprintf(s_responses[index], DATA_MAX_RESPONSE_LENGTH, "%s", response);

  // Every response we get is valid
  s_response_valids[index] = true;
}

char* data_get_response(int index) {
  return &s_responses[index][0];
}

bool data_get_contact_valid(ContactType type, int index) {
  switch(type) {
    case ContactTypeSMS:      return s_contact_valids[0];
    case ContactTypeCall:     return s_contact_valids[1];
    case ContactTypeIncoming: return s_contact_valids[index + 2];
    case ContactTypeFavorite: return s_contact_valids[index + 7];
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown data_get_contact_known query: %d %d", (int)type, index);
      return false;
  }
}

void data_set_total_responses(int total_responses) {
  s_total_responses = total_responses;
}

int data_get_total_responses() {
  return s_total_responses;
}

void data_set_obtained_responses(int obtained_responses) {
  s_obtained_responses = obtained_responses;
}

int data_get_obtained_responses() {
  return s_obtained_responses;
}

bool data_get_response_valid(int index) {
  return s_response_valids[index];
}
