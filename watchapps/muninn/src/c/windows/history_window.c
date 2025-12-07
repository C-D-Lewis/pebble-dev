#include "history_window.h"

#define ROW_HEIGHT 66
#define FONT_KEY_S FONT_KEY_GOTHIC_14
#define FONT_KEY_M FONT_KEY_GOTHIC_18
#define FONT_KEY_L FONT_KEY_GOTHIC_24
#define ROW_1_Y 20
#define ROW_2_Y 40
#define STATUS_W 56

#if defined(PBL_PLATFORM_EMERY)
  #define TITLE_FONT_KEY FONT_KEY_GOTHIC_24
  #define MENU_INSET 28
  #define DIV_X 140
#else
  #define TITLE_FONT_KEY FONT_KEY_GOTHIC_18
  #define MENU_INSET 20
  #define DIV_X 90
#endif

static Window *s_window;
static TextLayer *s_header_layer;
static MenuLayer *s_menu_layer;

static GFont s_font_s, s_font_m, s_font_l;

static void draw_diffs(GContext *ctx, const GRect bounds, const Sample *s) {
  // Time diff
  static char s_fmt_lst_buff[8];
  static char s_fmt_ts_buff[8];
  util_fmt_time(s->last_sample_time, &s_fmt_lst_buff[0], sizeof(s_fmt_lst_buff));
  util_fmt_time(s->timestamp, &s_fmt_ts_buff[0], sizeof(s_fmt_ts_buff));
  static char s_lst_buff[32];
  snprintf(s_lst_buff, sizeof(s_lst_buff), "%s -> %s", s_fmt_lst_buff, s_fmt_ts_buff);
  graphics_draw_text(
    ctx,
    s_lst_buff,
    s_font_m,
    GRect(2, ROW_1_Y, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Charge diff
  static char s_lcp_buff[32];
  snprintf(s_lcp_buff, sizeof(s_lcp_buff), "%d%% -> %d%%", s->last_charge_perc, s->charge_perc);
  graphics_draw_text(
    ctx,
    s_lcp_buff,
    s_font_m,
    GRect(2, ROW_2_Y, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
}

static void draw_status(GContext *ctx, const GRect bounds, const Sample *s, char *msg) {
  graphics_draw_text(
    ctx,
    msg,
    s_font_m,
    GRect(bounds.size.w - STATUS_W, ROW_1_Y, STATUS_W, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return data_get_log_length();
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const GRect bounds = layer_get_bounds(cell_layer);
  const int count = data_get_valid_samples_count();

  // Menu guards against this, but cover the case anyway
  if (count == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "No samples yet", NULL, NULL);
    return;
  }

  const int index = cell_index->row;
  const SampleData *data = data_get_sample_data();
  const Sample *s = &data->samples[index];

  // Sample time
  static char s_datetime_buff[32];
  const time_t ts_time = s->timestamp;
  const struct tm *ts_info = localtime(&ts_time);
  strftime(s_datetime_buff, sizeof(s_datetime_buff), "%Y-%m-%d - %H:%M", ts_info);
  static char s_dt_with_cp_buff[32];
  snprintf(
    s_dt_with_cp_buff,
    sizeof(s_dt_with_cp_buff),
    "%s: %d%%",
    s_datetime_buff,
    s->charge_perc
  );
  graphics_draw_text(
    ctx,
    s_dt_with_cp_buff,
    s_font_s,
    GRect(2, 0, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Divider
  const GColor sep_color = menu_layer_is_index_selected(s_menu_layer, cell_index)
    ? GColorWhite : GColorBlack;
  graphics_context_set_stroke_color(ctx, sep_color);
  graphics_draw_line(ctx, GPoint(0, 18), GPoint(bounds.size.w, 18));
  graphics_draw_line(ctx, GPoint(DIV_X, 18), GPoint(DIV_X, bounds.size.h));
  graphics_draw_line(ctx, GPoint(0, bounds.size.h - 1), GPoint(bounds.size.w, bounds.size.h - 1));

  draw_diffs(ctx, bounds, s);

  if (s->result == STATUS_NO_CHANGE) {
    draw_status(ctx, bounds, s, "No change");
  } else if (s->result == STATUS_CHARGED) {
    draw_status(ctx, bounds, s, "Charged up");
  } else {
    const char *str = s->result > 99 ? "Est.\n%d%%/d" : "Est.\n%d%%/d";
    static char s_result_buff[16];
    snprintf(s_result_buff, sizeof(s_result_buff), str, s->result);
    draw_status(ctx, bounds, s, &s_result_buff[0]);
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return ROW_HEIGHT;
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_header_layer = util_make_text_layer(
    GRect(0, -3, bounds.size.w, 32),
    fonts_get_system_font(TITLE_FONT_KEY)
  );
  text_layer_set_text(s_header_layer, "Data Log");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  s_font_s = fonts_get_system_font(FONT_KEY_S);
  s_font_m = fonts_get_system_font(FONT_KEY_M);
  s_font_l = fonts_get_system_font(FONT_KEY_L);

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

void history_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
