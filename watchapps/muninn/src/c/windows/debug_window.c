#include "debug_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define FONT_KEY FONT_KEY_GOTHIC_18
  #define HISTORY_Y 115
#else
  #define FONT_KEY FONT_KEY_GOTHIC_14
  #define HISTORY_Y 95
#endif

static Window *s_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_stats_layer;

void debug_window_update_data() {
  if (!s_window) return;

  const int wakeup_id = data_get_wakeup_id();
  time_t wakeup_ts;
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);
  const bool is_enabled = util_is_not_status(wakeup_id) && found;

  // AppState
  static char s_fmt_last_buff[8];
  util_fmt_time(data_get_last_sample_time(), &s_fmt_last_buff[0], sizeof(s_fmt_last_buff));
  static char s_fmt_next_buff[8];
  util_fmt_time(wakeup_ts, &s_fmt_next_buff[0], sizeof(s_fmt_next_buff));

  static char s_values_buff[100];
  snprintf(
    s_values_buff,
    sizeof(s_values_buff),
    "Scheduled: %s\nLast sample: %s\nNext sample: %s\nLast value: %d\nAccuracy: %d%%",
    is_enabled ? "true": "false",
    &s_fmt_last_buff[0],
    &s_fmt_next_buff[0],
    data_get_last_charge_perc(),
    data_calculate_accuracy()
  );
  text_layer_set_text(s_stats_layer, s_values_buff);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  GFont font = fonts_get_system_font(FONT_KEY);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(root_layer, status_bar_layer_get_layer(s_status_bar));

  s_stats_layer = util_make_text_layer(
    GRect(5, STATUS_BAR_LAYER_HEIGHT, PS_DISP_W - 10, PS_DISP_H),
    font
  );
  layer_add_child(root_layer, text_layer_get_layer(s_stats_layer));
}

static void window_unload(Window *window) {
  status_bar_layer_destroy(s_status_bar);

  text_layer_destroy(s_stats_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void debug_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  debug_window_update_data();
}
