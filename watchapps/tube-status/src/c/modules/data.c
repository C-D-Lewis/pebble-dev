#include "data.h"

static LineConfig s_line_configs[MAX_LINES];
static LineData s_line_data[MAX_LINES];
static int s_progress = 0;
static int s_progress_max = MAX_LINES * 2; // Config and Status are sent separately

void data_init() {
}

void data_deinit() {
}

void data_set_line_config(int index, const char *name, uint32_t color, bool striped) {
  if (index < 0 || index >= MAX_LINES) {
    return;
  }

  LineConfig *config = &s_line_configs[index];
  config->configured = true;
  snprintf(config->name, sizeof(config->name), "%s", name);
  config->color = color;
  config->striped = striped;
}

char *data_get_line_name(int index) {
  if (index < 0 || index >= MAX_LINES || !s_line_configs[index].configured) {
    return "?";
  }
  return s_line_configs[index].name;
}

GColor data_get_line_color(int index) {
#if defined(PBL_COLOR)
  if (index < 0 || index >= MAX_LINES || !s_line_configs[index].configured) {
    return GColorWhite;
  }
  return GColorFromHEX(s_line_configs[index].color);
#endif
  return GColorBlack;
}

LineData *data_get_line(int index) {
  if (index < 0 || index >= MAX_LINES) {
    return NULL;
  }
  return &s_line_data[index];
}

GColor data_get_line_state_color(int index) {
  if (index < 0 || index >= MAX_LINES) {
    return GColorClear;
  }

  StatusSeverity severity = s_line_data[index].severity;

  switch (severity) {
  case StatusSeverityWarning:
    return PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorDarkGray);
  case StatusSeveritySevere:
    return PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray);
  default:
    return GColorClear;
  }
}

bool data_get_line_color_is_striped(int index) {
  if (index < 0 || index >= MAX_LINES || !s_line_configs[index].configured) {
    return false;
  }
  return s_line_configs[index].striped;
}

void data_set_progress(int progress) {
  s_progress = progress;
}

int data_get_progress() {
  return s_progress;
}

bool data_get_line_has_reason(int index) {
  if (index < 0 || index >= MAX_LINES) {
    return false;
  }
  char *reason = data_get_line(index)->reason;
  return strlen(reason) != 0;
}

void data_set_progress_max(int max) {
  s_progress_max = max;
}

int data_get_progress_max() {
  return s_progress_max;
}

int data_get_lines_received() {
  int count = 0;
  for (int i = 0; i < MAX_LINES; i++) {
    if (strlen(s_line_data[i].state) != 0) {
      count++;
    }
  }
  return count;
}

int data_get_configured_line_count() {
  int count = 0;
  for (int i = 0; i < MAX_LINES; i++) {
    if (s_line_configs[i].configured) {
      count++;
    }
  }
  return count;
}
