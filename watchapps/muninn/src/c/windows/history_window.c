#include "history_window.h"

static Window *s_window;
static TextLayer *s_header_layer;
static MenuLayer *s_menu_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  const int count = data_get_samples_count();
  return count == 0 ? 1 : count;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const int count = data_get_samples_count();
  if (count == 0) {
    menu_cell_basic_draw(
      ctx,
      cell_layer,
      "No samples yet",
      NULL,
      NULL
    );
    return;
  }

  const int index = cell_index->row;

  const int value = data_get_sample_data()->values[index];
  static char s_level_buff[16];
  snprintf(s_level_buff, sizeof(s_level_buff), "~ %d%% / day", value);

  static char s_time_buff[32];
  const time_t sample_time = data_get_sample_data()->timestamps[index];
  const struct tm *tm_info = localtime(&sample_time);
  strftime(s_time_buff, sizeof(s_time_buff), "%b. %d %H:%M", tm_info);

  menu_cell_basic_draw(
    ctx,
    cell_layer,
    s_level_buff,
    s_time_buff,
    NULL
  );
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return 44;
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_header_layer = util_make_text_layer(
    GRect(0, -3, bounds.size.w, 24),
    fonts_get_system_font(FONT_KEY_GOTHIC_18)
  );
  text_layer_set_text(s_header_layer, "Recent Estimations");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(20, 0, 0, 0)));
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
