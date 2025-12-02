#include "stat_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define FONT_KEY FONT_KEY_GOTHIC_18
  #define HISTORY_Y 115
#else
  #define FONT_KEY FONT_KEY_GOTHIC_14
  #define HISTORY_Y 95
#endif

static Window *s_window;
static StatusBarLayer *s_status_bar;
static TextLayer *s_stats_layer, *s_history_layer;

void stat_window_update_data() {
  if (!s_window) return;

  const bool is_enabled = util_is_valid(data_get_wakeup_id());
  time_t wakeup_ts;
  wakeup_query(data_get_wakeup_id(), &wakeup_ts);

  // Current stats
  static char s_fmt_last_buff[16];
  util_fmt_time(data_get_last_sample_time(), &s_fmt_last_buff[0], sizeof(s_fmt_last_buff));

  static char s_fmt_next_buff[16];
  util_fmt_time(wakeup_ts, &s_fmt_next_buff[0], sizeof(s_fmt_next_buff));

  static char s_values_buff[128];
  snprintf(
    s_values_buff,
    sizeof(s_values_buff),
    "Enabled: %s\nLast sample: %s\nNext sample: %s\nLast value: %d",
    is_enabled ? "true": "false",
    &s_fmt_last_buff[0],
    &s_fmt_next_buff[0],
    data_get_last_charge_perc()
  );
  text_layer_set_text(s_stats_layer, s_values_buff);

  // Recent history
  SampleData *sample_data = data_get_sample_data();
  static char s_history_buff[512];
  snprintf(
    s_history_buff,
    sizeof(s_history_buff),
    "Recent samples:\n%d, %d, %d, %d, %d, %d\nAverage: %d",
    sample_data->samples[0].result,
    sample_data->samples[1].result,
    sample_data->samples[2].result,
    sample_data->samples[3].result,
    sample_data->samples[4].result,
    sample_data->samples[5].result,
    data_calculate_avg_discharge_rate()
  );
  text_layer_set_text(s_history_layer, s_history_buff);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  GFont font = fonts_get_system_font(FONT_KEY);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(root_layer, status_bar_layer_get_layer(s_status_bar));

  s_stats_layer = util_make_text_layer(
    GRect(5, STATUS_BAR_LAYER_HEIGHT, DISPLAY_W - 10, 168),
    font
  );
  layer_add_child(root_layer, text_layer_get_layer(s_stats_layer));

  s_history_layer = util_make_text_layer(
    GRect(5, STATUS_BAR_LAYER_HEIGHT + HISTORY_Y, DISPLAY_W - 10, 168),
    font
  );
  layer_add_child(root_layer, text_layer_get_layer(s_history_layer));
}

static void window_unload(Window *window) {
  status_bar_layer_destroy(s_status_bar);

  text_layer_destroy(s_stats_layer);
  text_layer_destroy(s_history_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void stat_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  stat_window_update_data();
}
