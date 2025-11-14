#include "stat_window.h"

static Window *s_window;
static TextLayer *s_enabled_layer, *s_stats_layer, *s_history_layer;

static void update_data() {
  // Is enabled?
  bool is_enabled = data_get_wakeup_id() != DATA_EMPTY;
  static char s_enabled_buff[16];
  snprintf(s_enabled_buff, sizeof(s_enabled_buff), "Enabled: %s", is_enabled ? "Yes" : "No");
  text_layer_set_text(s_enabled_layer, s_enabled_buff);

  time_t wakeup_ts;
  wakeup_query(data_get_wakeup_id(), &wakeup_ts);

  // Current stats
  static char s_current_buff[128];
  static char s_fmt_discharge_buff[8];
  static char s_fmt_update_buff[8];
  static char s_fmt_wakeup_buff[8];
  util_fmt_time(data_get_discharge_start_time(), &s_fmt_discharge_buff[0], sizeof(s_fmt_discharge_buff));
  util_fmt_time(data_get_last_update_time(), &s_fmt_update_buff[0], sizeof(s_fmt_update_buff));
  util_fmt_time(wakeup_ts, &s_fmt_wakeup_buff[0], sizeof(s_fmt_wakeup_buff));
  snprintf(
    s_current_buff,
    sizeof(s_current_buff),
    "S: %s\nU: %s\nW: %s\nB: %d, P: %s",
    &s_fmt_discharge_buff[0],
    &s_fmt_update_buff[0],
    &s_fmt_wakeup_buff[0],
    data_get_last_charge_perc(),
    data_get_was_plugged() ? "true": "false"
  );
  text_layer_set_text(s_stats_layer, s_current_buff);

  // Recent history
  SampleData *sample_data = data_get_sample_data();
  static char s_history_buff[64];
  snprintf(
    s_history_buff,
    sizeof(s_history_buff),
    "H: %d, %d, %d, %d, %d, %d\n-> avg: %d",
    sample_data->history[0],
    sample_data->history[1],
    sample_data->history[2],
    sample_data->history[3],
    sample_data->history[4],
    sample_data->history[5],
    data_get_history_avg_rate()
  );
  text_layer_set_text(s_history_layer, s_history_buff);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  GFont sys_24 = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GFont sys_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  // Enable toggle
  s_enabled_layer = util_make_text_layer(GRect(5, 5, WIDTH, 168), sys_24);
  layer_add_child(root_layer, text_layer_get_layer(s_enabled_layer));

  // Current stats (pending UI)
  s_stats_layer = util_make_text_layer(GRect(5, 30, WIDTH - 10, 168), sys_18);
  layer_add_child(root_layer, text_layer_get_layer(s_stats_layer));

  s_history_layer = util_make_text_layer(GRect(5, 100, WIDTH - 10, 168), sys_18);
  layer_add_child(root_layer, text_layer_get_layer(s_history_layer));

  // IF calculated avg rate is 0, show it's still being gathered

  // Average discharge rate

  // Time since last charge

  // Estimated time remaining (days / hours)
}

static void window_unload(Window *window) {
  text_layer_destroy(s_stats_layer);
  text_layer_destroy(s_enabled_layer);
  text_layer_destroy(s_history_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool should_enable = data_get_wakeup_id() == DATA_EMPTY;

  if (should_enable) {
    data_prepare();
    data_sample_now();
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

static void battery_handler(BatteryChargeState state) {
  // Un/plugged while app is open
  if (data_get_was_plugged() && !state.is_plugged) {
    const time_t now = time(NULL);
    data_set_discharge_start_time(now);
  }

  data_set_was_plugged(state.is_plugged);

  update_data();
}

void stat_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(s_window, click_config_provider);
  }

  window_stack_push(s_window, true);

  // If app is open, get more accurate battery data
  battery_state_service_subscribe(battery_handler);
  battery_handler(battery_state_service_peek());

  update_data();
}
