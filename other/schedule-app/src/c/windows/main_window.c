#include "main_window.h"

#define CELL_HEIGHT 54

static Window *s_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_bar;

/********************************* MenuLayer **********************************/

void draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  GRect bounds = layer_get_bounds(cell_layer);
  const int index = cell_index->row;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  const int text_x = 18;
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx,
    EVENT_NAMES[index],
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    GRect(text_x, -2, bounds.size.w, bounds.size.h),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_text(ctx,
    EVENT_TIMES[index],
    fonts_get_system_font(FONT_KEY_GOTHIC_24),
    GRect(text_x, 22, bounds.size.w, bounds.size.h),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  if (menu_layer_is_index_selected(s_menu_layer, cell_index)) {
    graphics_context_set_fill_color(ctx, GColorIslamicGreen);
  } else {
    graphics_context_set_fill_color(ctx, GColorVividCerulean);
  }
  graphics_fill_rect(ctx, GRect(0, 0, 10, CELL_HEIGHT), 0, GCornerNone);
}

uint16_t get_num_rows_handler(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return ARRAY_LENGTH(EVENT_NAMES);
}

int16_t get_cell_height_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return CELL_HEIGHT;
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  GEdgeInsets menu_insets = (GEdgeInsets) { .top = STATUS_BAR_LAYER_HEIGHT - 1 };
  GRect menu_bounds = grect_inset(bounds, menu_insets);

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  s_menu_layer = menu_layer_create(menu_bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  menu_layer_pad_bottom_enable(s_menu_layer, false);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .draw_row = draw_row_handler,
    .get_num_rows = get_num_rows_handler,
    .get_cell_height = get_cell_height_handler
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  status_bar_layer_destroy(s_status_bar);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
