#include "menu_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

typedef enum {
  MI_SETTINGS = 0,
  MI_BATTERY_TIPS,
  MI_INFORMATION,

  MI_MAX,
} MenuItems;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_SETTINGS:
      util_menu_cell_draw(ctx, cell_layer, "Settings", NULL);
      break;
    case MI_BATTERY_TIPS:
      util_menu_cell_draw(ctx, cell_layer, "Battery tips", NULL);
      break;
    case MI_INFORMATION:
      util_menu_cell_draw(ctx, cell_layer, "Information", NULL);
      break;
    default: break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return ROW_HEIGHT_SMALL;
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_SETTINGS:
      settings_window_push();
      break;
    case MI_BATTERY_TIPS:
      message_window_push(MSG_TIPS, false, false);
      break;
    case MI_INFORMATION:
      message_window_push(MSG_INFORMATION, false, false);
      break;
    default: break;
  }

  menu_layer_reload_data(s_menu_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
    .select_click = (MenuLayerSelectCallback)select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  window_destroy(window);
  s_window = NULL;
}

void menu_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
