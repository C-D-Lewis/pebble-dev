#include "log_window.h"

#define ROW_HEIGHT scl_y(390)
#define DIV_Y scl_y_pp({.o = 115, .e = 105})
#define X_START scl_x_pp({.o = 20, .c = 70, .e = 20})
#define X_END scl_x_pp({.o = 980, .c = 930, .e = 20})

// Extra precision needed
#if defined(PBL_PLATFORM_EMERY)
  #define DIV_W 2
#else
  #define DIV_W 1
#endif

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer *s_header_layer;
static MenuLayer *s_menu_layer;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Title underline
  graphics_fill_rect(ctx, GRect(0, scl_y(110), PS_DISP_W, LINE_W), 0, GCornerNone);
}

static void draw_changes(GContext *ctx, const GRect bounds, const Sample *s) {
  // Adjust timestamp back by a minute so it's within the interval
  int adj_ts = s->timestamp - SECONDS_PER_MINUTE;

  // Time diff
  static char s_fmt_lst_buff[8];
  static char s_fmt_ts_buff[8];
  util_fmt_time(s->last_sample_time, &s_fmt_lst_buff[0], sizeof(s_fmt_lst_buff));
  util_fmt_time(adj_ts, &s_fmt_ts_buff[0], sizeof(s_fmt_ts_buff));
  static char s_lst_buff[16];
  snprintf(s_lst_buff, sizeof(s_lst_buff), "%s -> %s", s_fmt_lst_buff, s_fmt_ts_buff);
  graphics_draw_text(
    ctx,
    s_lst_buff,
    scl_get_font(SFI_Medium),
    GRect(X_START, scl_y_pp({.o = 90, .e = 100}), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Charge diff
  static char s_lcp_buff[16];
  snprintf(s_lcp_buff, sizeof(s_lcp_buff), "%d%% -> %d%%", s->last_charge_perc, s->charge_perc);
  graphics_draw_text(
    ctx,
    s_lcp_buff,
    scl_get_font(SFI_Medium),
    GRect(X_START, scl_y_pp({.o = 200, .e = 210}), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Right-aligned amounts
  static char s_ts_diff_buff[8];
  util_fmt_time_unit(s->time_diff, &s_ts_diff_buff[0], sizeof(s_ts_diff_buff));
  graphics_draw_text(
    ctx,
    s_ts_diff_buff,
    scl_get_font(SFI_Medium),
    GRect(0, scl_y_pp({.o = 90, .e = 100}), X_END, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );

  static char s_perc_diff_buff[8];
  snprintf(s_perc_diff_buff, sizeof(s_perc_diff_buff), "%d%%", s->charge_diff);
  graphics_draw_text(
    ctx,
    s_perc_diff_buff,
    scl_get_font(SFI_Medium),
    GRect(0, scl_y_pp({.o = 200, .e = 210}), X_END, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
}

static void draw_result_and_datetime(GContext *ctx, const GRect bounds, const Sample *s) {
  static char s_datetime_buff[16];
  time_t ts_time = s->timestamp;

  // We subtract a minute so the date is within the majority of the interval
  ts_time -= SECONDS_PER_MINUTE;

  const struct tm *ts_info = localtime(&ts_time);
  strftime(s_datetime_buff, sizeof(s_datetime_buff), "%d %b", ts_info);

  static char s_result_buff[16];
  if (s->result == STATUS_NO_CHANGE) {
    snprintf(s_result_buff, sizeof(s_result_buff), "No change");
  } else if (s->result == STATUS_CHARGED) {
    snprintf(s_result_buff, sizeof(s_result_buff), "Charged up");
  } else {
    snprintf(s_result_buff, sizeof(s_result_buff), "Discharged");
  }

  graphics_draw_text(
    ctx,
    s_datetime_buff,
    scl_get_font(SFI_Small),
    GRect(X_START, scl_y(-25), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_text(
    ctx,
    s_result_buff,
    scl_get_font(SFI_Small),
    GRect(0, scl_y(-25), X_END, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  const int length = data_get_log_length();
  return length == 0 ? ROW_HEIGHT_SMALL : ROW_HEIGHT;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  const int length = data_get_log_length();
  return length == 0 ? 1 : length;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const GRect bounds = layer_get_bounds(cell_layer);
  const int count = data_get_log_length();

  if (count == 0) {
    graphics_draw_text(
      ctx,
      "No samples yet",
      scl_get_font(SFI_Medium),
      scl_grect(0, 20, 1000, 280),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
    );
    return;
  }

  const int index = cell_index->row;
  const Sample *s = data_get_sample(index);

  draw_result_and_datetime(ctx, bounds, s);

  // Divider
  const GColor sep_color = menu_layer_is_index_selected(s_menu_layer, cell_index)
    ? GColorWhite : GColorBlack;
  graphics_context_set_fill_color(ctx, sep_color);
  graphics_fill_rect(ctx, GRect(0, DIV_Y, bounds.size.w, 1), GCornerNone, 0);  // Always 1h
  graphics_fill_rect(ctx, GRect(0, bounds.size.h - DIV_W, bounds.size.w, DIV_W), GCornerNone, 0);

  draw_changes(ctx, bounds, s);
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_header_layer = util_make_text_layer(
    GRect(0, scl_y_pp({-30, .c = -20, .e = -25}), PS_DISP_W, 100),
    scl_get_font(SFI_Small)
  );
  text_layer_set_text(s_header_layer, PBL_IF_ROUND_ELSE("History", "Recent samples"));
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_header_layer));

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(HEADER_INSET, 0, 0, 0)));
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback
  });
  layer_add_child(root_layer, menu_layer_get_layer(s_menu_layer));

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  text_layer_destroy(s_header_layer);
  menu_layer_destroy(s_menu_layer);

  window_destroy(window);
  s_window = NULL;
}

void scroll_timer_handler(void *context) {
  int index = (int)context;
  MenuIndex next = { .row = index };
  menu_layer_set_selected_index(s_menu_layer, next, MenuRowAlignCenter, true);
}

void log_window_push(int index) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
  // APP_LOG(APP_LOG_LEVEL_INFO, "Heap %d", heap_bytes_free());

  if (index != 0) {
    app_timer_register(250, scroll_timer_handler, (void *)index);
  }
}
