#include "stats_window.h"

static Window *s_window;
static TextLayer *s_text_layer;

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_text_layer = util_make_text_layer(bounds, scl_get_font(SFI_Medium));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  text_layer_set_text(s_text_layer, "Loading...");
  layer_add_child(root_layer, text_layer_get_layer(s_text_layer));

  // Request data
  comm_request_sync_stats();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void stats_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);
}

void stats_window_reload() {
  AppState *app_state = data_get_app_state();

  // Display the stats
  // TODO: Nice layout with icons? As long as they are unloaded!
  static char s_buff[64];
  snprintf(
    s_buff,
    sizeof(s_buff),
    "%d days\n%d avg. rate",
    app_state->stat_total_days,
    app_state->stat_all_time_rate
  );
  text_layer_set_text(s_text_layer, s_buff);
}