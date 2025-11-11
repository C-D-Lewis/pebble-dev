#include "main_window.h"

#define WIDTH PBL_IF_ROUND_ELSE(180, 144)
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)

static Window *s_window;
static TextLayer *s_enabled_layer, *s_stats_layer;

static void update_data() {
  // Is enabled?
  bool is_enabled = data_get_wakeup_id() != NO_DATA;
  static char s_enabled_buff[16];
  snprintf(s_enabled_buff, sizeof(s_enabled_buff), "Enabled: %s", is_enabled ? "Yes" : "No");
  text_layer_set_text(s_enabled_layer, s_enabled_buff);

  // Current stats
  static char s_current_buff[128];
  snprintf(
    s_current_buff,
    sizeof(s_current_buff),
    "S: %d, U: %d, W: %d, P: %d, R: %d, C: %s",
    data_get_discharge_start_time(),
    data_get_last_update_time(),
    data_get_wakeup_id(),
    data_get_last_charge_perc(),
    data_get_discharge_rate(),
    data_get_was_plugged() ? "true": "false"
  );
  text_layer_set_text(s_stats_layer, s_current_buff);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  // Enable toggle
  s_enabled_layer = util_make_text_layer(GRect(10, 5, WIDTH, 168));
  layer_add_child(root_layer, text_layer_get_layer(s_enabled_layer));

  // Current stats (pending UI)
  s_stats_layer = util_make_text_layer(GRect(10, 30, WIDTH - 20, 168));
  layer_add_child(root_layer, text_layer_get_layer(s_stats_layer));

  // Time since last charge

  // Estimate time remaining
}

static void window_unload(Window *window) {
  text_layer_destroy(s_stats_layer);
  text_layer_destroy(s_enabled_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool new_state = !(data_get_wakeup_id() >= 0);

  if (new_state) {
    data_prepare();
    wakeup_schedule_next();
  } else {
    wakeup_unschedule();
  }

  update_data();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
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

  window_stack_push(s_window, true);
  update_data();
}
