#include "data.h"

// Data model
typedef struct {
  int type;
  int state;
} Toggle;

static Toggle s_toggles[ToggleTypeCount];
static char s_gsm_name[NETWORK_MAX_LENGTH];
static char s_wifi_name[NETWORK_MAX_LENGTH];
static char s_order[ToggleTypeCount];
static int s_gsm_percent, s_battery_percent, s_storage_gb_major, s_storage_gb_minor, s_storage_percent;
static bool s_is_lollipop;

void data_init() {
  for(int i = 0; i < ToggleTypeCount; i++) {
    s_toggles[i].type = i;
    s_toggles[i].state = ToggleStateWaiting;
  }

  snprintf(s_gsm_name, sizeof(s_gsm_name), "-");
  snprintf(s_wifi_name, sizeof(s_wifi_name), "-");
}

void data_deinit() { }

void data_set_toggle_type_at_index(int index, int type) {
  s_toggles[index].type = type;
}

int data_get_toggle_type_at_index(int index) {
  return s_toggles[index].type;
}

void data_set_toggle_state(int type, int state) {
  int index = 0;
  while(s_toggles[index].type != type) {
    index++;

    if(index == ToggleTypeCount) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "No toggle found with type %d!", type);
    }
  }
  s_toggles[index].state = state;
}

int data_get_toggle_state(int type) {
  int index = 0;
  while(s_toggles[index].type != type) {
    index++;

    if(index == ToggleTypeCount) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "No toggle found with type %d!", type);
    }
  }
  return s_toggles[index].state;
}

void data_set_gsm_name(char *name) {
  snprintf(s_gsm_name, sizeof(s_gsm_name), "%s", name);
}

char* data_get_gsm_name() {
  return &s_gsm_name[0];
}

void data_set_wifi_name(char *name) {
  snprintf(s_wifi_name, sizeof(s_wifi_name), "%s", name);
}

char* data_get_wifi_name() {
  return &s_wifi_name[0];
}

void data_set_gsm_percent(int percent) {
  s_gsm_percent = percent;
}

int data_get_gsm_percent() {
  return s_gsm_percent;
}

void data_set_battery_percent(int percent) {
  s_battery_percent = percent;
}

int data_get_battery_percent() {
  return s_battery_percent;
}

void data_set_storage_gb_major(int value) {
  s_storage_gb_major = value;
}

int data_get_storage_gb_major() {
  return s_storage_gb_major;
}

void data_set_storage_gb_minor(int value) {
  s_storage_gb_minor = value;
}

int data_get_storage_gb_minor() {
  return s_storage_gb_minor;
}

void data_set_is_lollipop(bool is_lollipop) {
  s_is_lollipop = is_lollipop;
  persist_write_bool(PersistKeyIsLollipop, is_lollipop);
}

bool data_get_is_lollipop() {
  return false; // Ignore for now. Is it even a thing anymore?
  // return s_is_lollipop;
}

static int char2int(char c) {
  return (int)(c - 48);
}

void data_set_toggle_order(char *order) {
  snprintf(s_order, sizeof(s_order), "%s", order);
  for(int i = 0; i < ToggleTypeCount; i++) {
    s_toggles[i].type = char2int(order[i]);
    s_toggles[i].state = ToggleStateWaiting;

    if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Set s_toggles[%d] to type %d", i, s_toggles[i].type);
  }
}

char* data_get_toggle_order() {
  return &s_order[0];
}

void data_set_storage_percent(int perc) {
  s_storage_percent = perc;
}

int data_get_storage_percent() {
  return s_storage_percent;
}

int data_get_toggle_index_with_type(int type) {
  int index = 0;
  while(s_toggles[index].type != type) {
    index++;
    
    if(index == ToggleTypeCount) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "No toggle found with type %d!", type);
    }
  }
  return index;
}
