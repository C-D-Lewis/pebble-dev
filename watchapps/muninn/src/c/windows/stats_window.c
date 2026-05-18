#include "stats_window.h"

#ifdef FEATURE_SYNC

typedef enum {
  MI_TOTAL_DURATION,
  MI_ALL_TIME_RATE,
  MI_LAST_WEEK_RATE,
  MI_EST_BATTERY_LIFE,
  MI_MTBC,
  MI_UPLOAD,

  MI_MAX,
} MenuItems;

static Window *s_window;
static MenuLayer *s_menu_layer;
static Layer *s_header_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  AppState *app_state = data_get_app_state();

  // Format buffers
  static char s_t_d_buff[32];
  if (util_is_not_status(app_state->stat_total_days)) {
    snprintf(s_t_d_buff, sizeof(s_t_d_buff), "%d days", app_state->stat_total_days);
  } else {
    snprintf(s_t_d_buff, sizeof(s_t_d_buff), "-");
  }
  // snprintf(s_t_d_buff, sizeof(s_t_d_buff), "16 days (4%%)");

  static char s_a_t_r_buff[16];
  if (
    strlen(app_state->stat_all_time_rate) != 0 &&
    strcmp(app_state->stat_all_time_rate, "-1") != 0
  ) {
    snprintf(s_a_t_r_buff, sizeof(s_a_t_r_buff), "%s%% per day", app_state->stat_all_time_rate);
  } else {
    snprintf(s_a_t_r_buff, sizeof(s_a_t_r_buff), "-");
  }
  // snprintf(s_a_t_r_buff, sizeof(s_a_t_r_buff), "5%% per day");

  static char s_l_w_r_buff[16];
  if (
    strlen(app_state->stat_last_week_rate) != 0 &&
    strcmp(app_state->stat_last_week_rate, "-1") != 0
  ) {
    snprintf(s_l_w_r_buff, sizeof(s_l_w_r_buff), "%s%% per day", app_state->stat_last_week_rate);
  } else {
    snprintf(s_l_w_r_buff, sizeof(s_l_w_r_buff), "-");
  }
  // snprintf(s_l_w_r_buff, sizeof(s_l_w_r_buff), "8%% per day");

  static char s_e_b_l_buff[16];
  if (util_is_not_status(app_state->stat_battery_life)) {
    snprintf(s_e_b_l_buff, sizeof(s_e_b_l_buff), "%d days", app_state->stat_battery_life);
  } else {
    snprintf(s_e_b_l_buff, sizeof(s_e_b_l_buff), "-");
  }
  // snprintf(s_e_b_l_buff, sizeof(s_e_b_l_buff), "20 days");

  static char s_mtbc_buff[16];
  if (util_is_not_status(app_state->stat_mtbc)) {
    snprintf(s_mtbc_buff, sizeof(s_mtbc_buff), "%d days", app_state->stat_mtbc);
  } else {
    snprintf(s_mtbc_buff, sizeof(s_mtbc_buff), "Req. 2 charges");
  }
  // snprintf(s_mtbc_buff, sizeof(s_mtbc_buff), "12 days");

  switch(cell_index->row) {
    case MI_TOTAL_DURATION: {
      const GColor fg_color = menu_cell_layer_is_highlighted(cell_layer) ? GColorWhite : GColorBlack;
      graphics_context_set_text_color(ctx, fg_color);
      graphics_draw_text(
        ctx,
        s_t_d_buff,
        scl_get_font(SFI_Medium),
        GRect(scl_x_pp({.o = 30, .c = 60}), scl_y_pp({.o = 5, .e = 22}), PS_DISP_W, 50),
        GTextOverflowModeTrailingEllipsis,
        GTextAlignmentLeft,
        NULL
      );

      // Filling up rect progressbar
      const int bar_x = scl_x_pp({.o = 480, .c = 450});
      const int bar_y = scl_y(30);
      const int bar_w = scl_x(490);
      const int bar_h = scl_y(70);
      const int sync_perc = (app_state->sync_count * 100) / MAX_SYNC_ITEMS;
      graphics_context_set_stroke_color(ctx, fg_color);
      graphics_draw_rect(ctx, GRect(bar_x, bar_y, bar_w, bar_h));
      graphics_context_set_fill_color(ctx, fg_color);
      graphics_fill_rect(
        ctx,
        GRect(bar_x, bar_y, bar_w * sync_perc / 100, bar_h),
        0,
        GCornerNone
      );

      // Percentage text label beneath progress bar
      char perc_buff[10];
      snprintf(perc_buff, sizeof(perc_buff), "%d%% full", sync_perc);
      graphics_draw_text(
        ctx,
        perc_buff,
        scl_get_font(SFI_Small),
        GRect(bar_x, bar_y + bar_h - scl_y(18), bar_w, 30),
        GTextOverflowModeTrailingEllipsis,
        GTextAlignmentCenter,
        NULL
      );
    } break;
    case MI_ALL_TIME_RATE:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Average Rate",
        s_a_t_r_buff
      );
      break;
    case MI_LAST_WEEK_RATE:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Weekly Avg. Rate",
        s_l_w_r_buff
      );
      break;
    case MI_EST_BATTERY_LIFE:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Est. Battery Life",
        s_e_b_l_buff
      );
      break;
    case MI_MTBC:
      util_menu_cell_draw(
        ctx,
        cell_layer,
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
        "Avg. Charge Interval",
#else
        "Avg. Charge Gap",
#endif
        s_mtbc_buff
      );
      break;
    case MI_UPLOAD:
      graphics_draw_text(
        ctx,
#ifdef PBL_PLATFORM_CHALK
        "Open Muninn mobile\nSettings for all data",
#else
        "Open Muninn mobile app Settings to see all data",
#endif
        scl_get_font(SFI_Small),
        GRect(0, scl_y_pp({.o = 0, .c = -20, .g = -20}), PS_DISP_W, 100),
        GTextOverflowModeTrailingEllipsis,
        GTextAlignmentCenter,
        NULL
      );
      break;
    default: break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_TOTAL_DURATION:
      return ROW_HEIGHT_SMALL;
    default:
      return ROW_HEIGHT_LARGE;
  }
}

void selection_will_change_callback(struct MenuLayer *menu_layer, MenuIndex *new_index, MenuIndex old_index, void *context) {
  // Last item isn't selectable
  if (new_index->row == MI_MAX - 1) {
    new_index->row = old_index.row;
  }
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_header_layer = util_create_header_layer(PBL_IF_ROUND_ELSE("Sync Stats", "Phone Sync Stats"), 32);
  layer_add_child(root_layer, s_header_layer);

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(HEADER_INSET, 0, 0, 0)));
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
    .selection_will_change = selection_will_change_callback,
  });
  layer_add_child(root_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  layer_destroy(s_header_layer);
  menu_layer_destroy(s_menu_layer);

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
  
  menu_layer_reload_data(s_menu_layer);
}

#endif
