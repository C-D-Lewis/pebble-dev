#include "main_window.h"

#define CELL_HEIGHT 54

static Window *s_window;
static TextLayer *s_text_layer;

static int s_breaths = 10;

static void schedule_next_wakeup() {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  // Schedule for an hour from now, next 30m past the hour
  tick_time->tm_min = 30;
  tick_time->tm_sec = 0;
  tick_time->tm_hour += 1;

  time_t next_wakeup = mktime(tick_time);
  int id = wakeup_schedule(next_wakeup, 0, true);

  if (id < 0 && s_text_layer != NULL) {
    text_layer_set_text(s_text_layer, "Failed to schedule wakeup");
  }
}

static void timer_handler(void *context) {
  window_stack_pop_all(true);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_breaths--;

  static char s_buff[128];
  snprintf(s_buff, sizeof(s_buff), "Breaths remaining:\n%d", s_breaths);
  text_layer_set_text(s_text_layer, s_buff);

  if (s_breaths == 0) {
    vibes_double_pulse();
    app_timer_register(1500, timer_handler, NULL);
    return;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(50, 0, 0, 0)));
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_text_layer, "Breaths remaining:\n10");
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(s_window, click_config_provider);
  }

  // Always schedule
  schedule_next_wakeup();  

  // Only wake between 9AM and 11PM
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  if (tick_time->tm_hour < 9 || tick_time->tm_hour >= 22) {
    window_stack_pop_all(true);
    return;
  }

  window_stack_push(s_window, true);
  vibes_long_pulse();
}
