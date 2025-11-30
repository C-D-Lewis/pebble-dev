#include "history_window.h"

#define ROW_HEIGHT 62
#define FONT_KEY_S FONT_KEY_GOTHIC_14
#define FONT_KEY_M FONT_KEY_GOTHIC_18
#define FONT_KEY_L FONT_KEY_GOTHIC_24
#define ROW_1_Y 16
#define ROW_2_Y 34

#if defined(PBL_PLATFORM_EMERY)
  #define FMT_STRING_ESTIMATE "About %d%% / day"
  #define TITLE_FONT_KEY FONT_KEY_GOTHIC_24
  #define MENU_INSET 28
#else
  #define FMT_STRING_ESTIMATE "~ %d%%/day"
  #define TITLE_FONT_KEY FONT_KEY_GOTHIC_18
  #define MENU_INSET 20
#endif

static Window *s_window;
static TextLayer *s_header_layer;
static MenuLayer *s_menu_layer;

static GFont s_font_s, s_font_m, s_font_l;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  const int count = data_get_samples_count();
  return count == 0 ? 1 : count;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const GRect bounds = layer_get_bounds(cell_layer);
  const int count = data_get_samples_count();

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
  const struct tm *ts_info = gmtime(&ts_time);
  strftime(s_datetime_buff, sizeof(s_datetime_buff), "%Y-%m-%d %H:%M", ts_info);
  static char s_dt_with_cp_buff[32];
  snprintf(
    s_dt_with_cp_buff,
    sizeof(s_dt_with_cp_buff),
    "%s - %d%%",
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

  // Last time
  static char s_fmt_lst_buff[16];
  util_fmt_time(s->last_sample_time, &s_fmt_lst_buff[0], sizeof(s_fmt_lst_buff));
  static char s_lst_buff[16];
  snprintf(s_lst_buff, sizeof(s_lst_buff), "<- %s", s_fmt_lst_buff);
  graphics_draw_text(
    ctx,
    s_lst_buff,
    s_font_m,
    GRect(2, ROW_1_Y, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Last charge
  static char s_lcp_buff[16];
  snprintf(s_lcp_buff, sizeof(s_lcp_buff), "<- %d%%", s->last_charge_perc);
  graphics_draw_text(
    ctx,
    s_lcp_buff,
    s_font_m,
    GRect(2, ROW_2_Y, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Time diff
  static char s_td_buff[32];
  static char s_fmt_td_buff[16];
  util_fmt_time_unit(s->time_diff, &s_fmt_td_buff[0], sizeof(s_fmt_td_buff));
  snprintf(s_td_buff, sizeof(s_td_buff), "dT: %s", s_fmt_td_buff);
  graphics_draw_text(
    ctx,
    s_td_buff,
    s_font_m,
    GRect(60, ROW_1_Y, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Charge diff
  static char s_cd_buff[16];
  snprintf(s_cd_buff, sizeof(s_cd_buff), "dC: %d%%", s->charge_diff);
  graphics_draw_text(
    ctx,
    s_cd_buff,
    s_font_m,
    GRect(60, ROW_2_Y, bounds.size.w, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Percentage per day
  graphics_draw_text(
    ctx,
    "Est.",
    s_font_s,
    GRect(0, 14, DISPLAY_W, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
  static char s_ppd_buff[8];
  snprintf(s_ppd_buff, sizeof(s_ppd_buff), "%d%%", s->perc_per_day);
  graphics_draw_text(
    ctx,
    s_ppd_buff,
    s_font_l,
    GRect(0, 21, DISPLAY_W, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
  graphics_draw_text(
    ctx,
    "/day",
    s_font_s,
    GRect(0, 43, DISPLAY_W, 28),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
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
