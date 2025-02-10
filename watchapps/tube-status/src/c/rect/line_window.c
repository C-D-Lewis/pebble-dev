#if defined(PBL_RECT)
#include "line_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static Layer *s_hint_layer;
static StatusBarLayer *s_status_bar;

static void hint_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int nudge = 3;
  const int radius = 15;
  GPoint center = GPoint(bounds.size.w + (radius / 2) - nudge, (bounds.size.h / 2) - nudge);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, radius);

  // Hold
  center.x -= (2 * radius) / 3;
  center.x--;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, 2);
}

/********************************* MenuLayer **********************************/

void draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) { 
  int type = cell_index->row;

  GRect bounds = layer_get_bounds(cell_layer);
  GRect logo_bounds = GRect(bounds.origin.x + (LINE_WINDOW_MARGIN / 2), 
    (bounds.size.h - (2 * LINE_WINDOW_RADIUS)) / 2, 
    (2 * LINE_WINDOW_RADIUS), (2 * LINE_WINDOW_RADIUS));
  GPoint center = grect_center_point(&logo_bounds);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, GCornerNone, 0);

  // Line color
  graphics_context_set_fill_color(ctx, data_get_line_color(type));
  graphics_fill_rect(ctx, GRect(center.x - (LINE_WINDOW_MARGIN / 2), bounds.origin.y, 
    LINE_WINDOW_MARGIN, bounds.size.h), GCornerNone, 0);

  // Draw circle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, LINE_WINDOW_RADIUS - 1);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, (5 * (LINE_WINDOW_RADIUS - 1)) / 7); 

  // Closed?
  graphics_context_set_fill_color(ctx, data_get_line_state_color(type));
  graphics_fill_circle(ctx, center, (5 * (LINE_WINDOW_RADIUS - 1)) / 7); 

  // Show selected
  if(menu_layer_is_index_selected(s_menu_layer, cell_index)) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, center, (4 * (LINE_WINDOW_RADIUS - 2)) / 7); 
  }

  // Info
  GRect text_bounds = GRect(bounds.origin.x + (3 * LINE_WINDOW_MARGIN), 
    bounds.origin.y - 5, bounds.size.w - (4 * LINE_WINDOW_MARGIN), 30);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, data_get_line_name(type), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    text_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
  text_bounds.origin.y += 25;
  text_bounds.size.w -= LINE_WINDOW_MARGIN;
  graphics_draw_text(ctx, data_get_line_state(type), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    text_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

  // Sep
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.size.h - 2, bounds.size.w, 2), GCornerNone, 0);
}

uint16_t get_num_rows_handler(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return LineTypeMax;
}

/******************************** Click Config ********************************/

static void select_click_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) { 
  settings_window_push();
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  GEdgeInsets menu_insets = (GEdgeInsets) {.top = STATUS_BAR_LAYER_HEIGHT};
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
    .select_long_click = select_click_handler
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_hint_layer = layer_create(bounds);
  layer_set_update_proc(s_hint_layer, hint_update_proc);
  layer_add_child(window_layer, s_hint_layer);
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  layer_destroy(s_hint_layer);
  status_bar_layer_destroy(s_status_bar);
  
  window_destroy(s_window);
  s_window = NULL;
  window_stack_pop_all(true);  // Don't show splash on exit
}

void line_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
#endif
