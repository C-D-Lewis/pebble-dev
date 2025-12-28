#include "log_window.h"

#define ROW_HEIGHT scalable_y(410)
#define ROW_HEIGHT_SMALL scalable_y(240)
#define DIV_Y scalable_y(125)
#define MENU_INSET scalable_y(135)

// Extra precision needed
#if defined(PBL_PLATFORM_EMERY)
  #define DIV_W 2
#else
  #define DIV_W 1
#endif

static Window *s_window;
static TextLayer *s_header_layer;
static MenuLayer *s_menu_layer;

static void draw_changes(GContext *ctx, const GRect bounds, const Sample *s) {
  // Adjust timestamp back by a minute so it's within the interval
  int adj_ts = s->timestamp - SECONDS_PER_MINUTE;

  // Time diff
  static char s_fmt_lst_buff[8];
  static char s_fmt_ts_buff[8];
  util_fmt_time(s->last_sample_time, &s_fmt_lst_buff[0], sizeof(s_fmt_lst_buff));
  util_fmt_time(adj_ts, &s_fmt_ts_buff[0], sizeof(s_fmt_ts_buff));
  static char s_lst_buff[32];
  snprintf(s_lst_buff, sizeof(s_lst_buff), "%s -> %s", s_fmt_lst_buff, s_fmt_ts_buff);
  graphics_draw_text(
    ctx,
    s_lst_buff,
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(20, 100, 1000, 280),
      GRect(20, 110, 1000, 280)
    ),
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
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(20, 210, 1000, 280),
      GRect(20, 220, 1000, 280)
    ),
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
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(0, 100, 980, 280),
      GRect(0, 110, 980, 280)
    ),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );

  static char s_perc_diff_buff[8];
  snprintf(s_perc_diff_buff, sizeof(s_perc_diff_buff), "%d%%", s->charge_diff);
  graphics_draw_text(
    ctx,
    s_perc_diff_buff,
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(0, 210, 980, 280),
      GRect(0, 220, 980, 280)
    ),
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
    snprintf(
      s_result_buff,
      sizeof(s_result_buff),
      "Est: %d%%/d",
      s->result
    );
  }

  graphics_draw_text(
    ctx,
    s_datetime_buff,
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(20, -50, 1000, 280),
      GRect(20, -30, 1000, 280)
    ),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_text(
    ctx,
    s_result_buff,
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(0, -50, 980, 280),
      GRect(0, -30, 980, 280)
    ),
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
      scalable_get_font(SFI_Medium),
      scalable_grect(0, 20, 1000, 280),
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
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const GRect header_rect = scalable_grect_pp(
    GRect(0, -40, 1000, 300),
    GRect(0, -20, 1000, 300)
  );

  s_header_layer = util_make_text_layer(header_rect, scalable_get_font(SFI_Medium));
  static char s_header_buff[32];
  snprintf(s_header_buff, sizeof(s_header_buff), "Data Log (%d/%d)", data_get_log_length(), NUM_SAMPLES);
  text_layer_set_text(s_header_layer, s_header_buff);
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(MENU_INSET, 0, 0, 0)));
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  menu_layer_destroy(s_menu_layer);

  window_destroy(window);
  s_window = NULL;
}

void log_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
