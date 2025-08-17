#include "health.h"

static bool s_health_available;
static int s_step_count = 0;

static void health_handler(HealthEventType event, void *context) {
  if(event != HealthEventSleepUpdate) {
    s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  }
}

void health_init() {
  s_health_available = health_service_events_subscribe(health_handler, NULL);

  if (s_health_available) {
      s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  }
}

bool is_health_available() {
    return s_health_available;
}

int get_step_count() {
    return s_step_count;
}

bool step_data_is_available() {
  return is_health_available() && (HealthServiceAccessibilityMaskAvailable &
  health_service_metric_accessible(HealthMetricStepCount,
    time_start_of_today(), time(NULL)));
}
