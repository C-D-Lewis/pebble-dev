#include "stats_window.h"

#ifdef FEATURE_SYNC

static Window *s_window;
static TextLayer *s_header_layer;
static ScrollLayer *s_scroll_layer;
static Layer *s_canvas_layer, *s_underline_layer;

static void underline_update_proc(Layer *layer, GContext *ctx) {
  // Title underline
  graphics_fill_rect(ctx, GRect(0, scl_y(110), PS_DISP_W, LINE_W), 0, GCornerNone);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();

  graphics_context_set_text_color(ctx, GColorBlack);

  if (!util_is_not_status(app_state->stat_total_days)) {
    graphics_draw_text(
      ctx,
      "Loading...",
      scl_get_font(SFI_Medium),
      GRect(0, scl_y(180), PS_DISP_W, 300),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL
    );
    return;
  }

  int y = scl_y(0);
  const int label_off = scl_y_pp({.o = 85, .e = 90});
  const int y_gap = scl_y_pp({.o = 240, .e = 230});

  // Total days
  graphics_draw_text(
    ctx,
    PBL_IF_ROUND_ELSE("Total duration", "Total duration synced"),
    scl_get_font(SFI_Small),
    GRect(0, y, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  static char s_t_d_buff[8];
  snprintf(s_t_d_buff, sizeof(s_t_d_buff), "%d days", app_state->stat_total_days);
  graphics_draw_text(
    ctx,
#ifdef USE_TEST_DATA
    "28 days",
#else
    s_t_d_buff,
#endif
    scl_get_font(SFI_Medium),
    GRect(0, y + label_off, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  y += y_gap;

  // All time avg
  graphics_draw_text(
    ctx,
    "Avg. discharge rate",
    scl_get_font(SFI_Small),
    GRect(0, y, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  static char s_a_t_r_buff[16];
  if (util_is_not_status(app_state->stat_all_time_rate)) {
    snprintf(s_a_t_r_buff, sizeof(s_a_t_r_buff), "%d%% per day", app_state->stat_all_time_rate);
  } else {
    snprintf(s_a_t_r_buff, sizeof(s_a_t_r_buff), "-");

  }
  graphics_draw_text(
    ctx,
#ifdef USE_TEST_DATA
    "12% per day",
#else
    s_a_t_r_buff,
#endif
    scl_get_font(SFI_Medium),
    GRect(0, y + label_off, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  y += y_gap;

  // Last week avg
  graphics_draw_text(
    ctx,
    "Last week avg. rate",
    scl_get_font(SFI_Small),
    GRect(0, y, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  static char s_l_w_r_buff[16];
  if (util_is_not_status(app_state->stat_last_week_rate)) {
    snprintf(s_l_w_r_buff, sizeof(s_l_w_r_buff), "%d%% per day", app_state->stat_last_week_rate);
  } else {
    snprintf(s_l_w_r_buff, sizeof(s_l_w_r_buff), "-");
  }
  graphics_draw_text(
    ctx,
#ifdef USE_TEST_DATA
    "8% per day",
#else
    s_l_w_r_buff,
#endif
    scl_get_font(SFI_Medium),
    GRect(0, y + label_off, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  y += y_gap;

  // Num charges
  graphics_draw_text(
    ctx,
    "Charge events",
    scl_get_font(SFI_Small),
    GRect(0, y, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  static char s_n_c_buff[12];
  snprintf(s_n_c_buff, sizeof(s_n_c_buff), "%d", app_state->stat_num_charges);
  graphics_draw_text(
    ctx,
#ifdef USE_TEST_DATA
    "3 events",
#else
    s_n_c_buff,
#endif
    scl_get_font(SFI_Medium),
    GRect(0, y + label_off, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  y += y_gap;

  // Mean time between charges
  graphics_draw_text(
    ctx,
    "Avg. charge interval",
    scl_get_font(SFI_Small),
    GRect(0, y, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  static char s_mtbc_buff[12];
  if (util_is_not_status(app_state->stat_mtbc)) {
    snprintf(s_mtbc_buff, sizeof(s_mtbc_buff), "%d days", app_state->stat_mtbc);
  } else {
    snprintf(s_mtbc_buff, sizeof(s_mtbc_buff), "-");
  }
  graphics_draw_text(
    ctx,
#ifdef USE_TEST_DATA
    "18 days",
#else
    s_mtbc_buff,
#endif
    scl_get_font(SFI_Medium),
    GRect(0, y + label_off, PS_DISP_W, 300),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  // TODO: Componentize this across settings, log, graph, stats windows
  s_header_layer = util_make_text_layer(
    GRect(0, scl_y_pp({-30, .c = -20, .e = -25}), PS_DISP_W, 100),
    scl_get_font(SFI_Small)
  );
  text_layer_set_text(s_header_layer, PBL_IF_ROUND_ELSE("Historical", "Historical stats"));
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_header_layer));

  s_underline_layer = layer_create(
    GRect(0, 0, PS_DISP_W, 100)
  );
  layer_set_update_proc(s_underline_layer, underline_update_proc);
  layer_add_child(root_layer, s_underline_layer);

  const int canvas_size = scl_y_pp({.o = 1300, .e = 1200});
  s_canvas_layer = layer_create(
    GRect(0, 0, PS_DISP_W, canvas_size)
  );
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  const int scroll_y = scl_y(110);
  s_scroll_layer = scroll_layer_create(
    GRect(0, scroll_y, bounds.size.w, bounds.size.h - scroll_y)
  );
  scroll_layer_set_content_size(s_scroll_layer, GSize(PS_DISP_W, canvas_size));
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_add_child(s_scroll_layer, s_canvas_layer);
  layer_add_child(root_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  layer_destroy(s_underline_layer);
  layer_destroy(s_canvas_layer);
  scroll_layer_destroy(s_scroll_layer);

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
  if (!s_window) return;
  
  layer_mark_dirty(s_canvas_layer);
}

#endif
