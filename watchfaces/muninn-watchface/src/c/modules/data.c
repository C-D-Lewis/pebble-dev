#include "data.h"

static AppState s_app_state;

void data_init() {
  s_app_state.days_remaining = STATUS_EMPTY;
  s_app_state.battery_life = STATUS_EMPTY;
  s_app_state.last_week_rate_x10 = STATUS_EMPTY;

#if defined(TEST)
  s_app_state.days_remaining = 12;
  s_app_state.battery_life = 16;
  s_app_state.last_week_rate_x10 = 38;
#endif
}

void data_deinit() {}

AppState* data_get_app_state() {
  return &s_app_state;
}
