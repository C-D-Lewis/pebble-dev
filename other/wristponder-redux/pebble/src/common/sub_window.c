#include "sub_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

// Could be Incoming or Favorites
static ContactType s_type;

/********************************* MenuLayer **********************************/

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return DATA_MAX_STREAMED_CONTACTS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
#if defined(PBL_COLOR)
  graphics_context_set_text_color(ctx, data_get_contact_valid(s_type, cell_index->row) ? GColorWhite : GColorDarkGray);
#endif
  menu_cell_basic_draw(ctx, cell_layer, data_get_contact_name(s_type, cell_index->row), NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT,
    44);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  int index = cell_index->row;

  if(data_get_contact_valid(s_type, index)) {
    responses_window_push(s_type, index);
  }
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, WRISTPONDER_THEME_COLOR, GColorWhite);
#endif
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void sub_window_push(ContactType type) {
  s_type = type;

  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}
