#include "wakeup_window.h"

static Window *s_window;
static TextLayer *s_title_layer, *s_details_layer;

static int s_type;
static int s_state;

static void timer_handler(void *context) {
  window_stack_pop_all(true);  // Quit wakeup
}

char* wakeup_window_get_state_string(int state) {
  switch(state) {
    case ToggleStateWaiting:              return "WAITING...";
    case ToggleStateOff:                  return "OFF";
    case ToggleStateOn:                   return "ON";
    case ToggleStateLoud:                 return "LOUD";
    case ToggleStateVibrate:              return "VIBRATE";
    case ToggleStateSilent:               return "SILENT";
    case ToggleStateBrightnessManual:     return "MANUAL";
    case ToggleStateBrightnessAutomatic:  return "AUTO";
    case ToggleStateLocked:               return "NOW LOCKED";
    default: 
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown state for wakeup_window_get_state_string: %d", state);
      return "UNKNOWN";
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_title_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(20, 0), bounds.size.w, 60));
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
  text_layer_set_text_color(s_title_layer, GColorWhite);
  text_layer_set_background_color(s_title_layer, GColorClear);
  text_layer_set_text(s_title_layer, "Dashboard Event");
  layer_add_child(window_layer, text_layer_get_layer(s_title_layer));

  s_details_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(80, 70), bounds.size.w, bounds.size.h));
  text_layer_set_font(s_details_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_details_layer, GColorWhite);
  text_layer_set_background_color(s_details_layer, GColorClear);
  text_layer_set_text_alignment(s_details_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_details_layer));

  static char s_buffer[128];
  snprintf(s_buffer, sizeof(s_buffer), "Attempting %s to %s\n(Back to dismiss)", 
    list_window_get_type_string (s_type), wakeup_window_get_state_string(s_state));
  text_layer_set_text(s_details_layer, s_buffer);

  // Two minutes
  const int two_mins_ms = 120000;
  app_timer_register(two_mins_ms, timer_handler, NULL);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_details_layer);

  // Finally
  window_destroy(window);
  s_window = NULL;
  window_stack_pop_all(true);
}

void wakeup_window_push(int type, int state) {
  s_type = type;
  s_state = state;

  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}
