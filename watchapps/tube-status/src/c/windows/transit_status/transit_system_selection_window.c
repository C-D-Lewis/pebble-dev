#if defined(APP_VARIANT_TRANSIT_STATUS)
#include "transit_system_selection_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_bar;

/******************************** Click config *********************************/
static void select_click_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  int index = (int)cell_index->row;
  data_set_progress(0); // Reset progress for new transit system load
  comm_request_transit_system(index);
  splash_window_push();
}

/********************************* MenuLayer **********************************/
static void draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  GRect bounds = layer_get_bounds(cell_layer);
  int index = cell_index->row;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, GCornerNone, 0);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    data_get_transit_system_region(index),
    scalable_get_font(SFI_Medium),
    scalable_grect_pp(
      GRect(20, -40, 930, 200),
      GRect(30, -20, 935, 200)
    ),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_text(
    ctx,
    data_get_transit_system_name(index),
    scalable_get_font(SFI_MediumBold),
    scalable_grect_pp(
      GRect(20, 75, 930, 200),
      GRect(30, 95, 935, 200)
    ),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  if (menu_layer_is_index_selected(s_menu_layer, cell_index)) {
    // Background
    graphics_context_set_fill_color(ctx, data_get_line_state_color(cell_index->row));
    graphics_fill_rect(
      ctx,
      scalable_grect(915, 0, 90, 260),
      GCornerNone,
      0
    );

    // Arrow
    graphics_draw_text(
      ctx,
      ">",
      scalable_get_font(SFI_MediumBold),
      scalable_grect_pp(
        GRect(930, 25, 120, 500),
        GRect(935, 40, 120, 500)
      ),
      GTextOverflowModeWordWrap,
      GTextAlignmentLeft,
      NULL
    );
  }
}

static uint16_t get_num_rows_handler(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MAX_TRANSIT_SYSTEMS;
}

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
    .select_click = select_click_handler,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  status_bar_layer_destroy(s_status_bar);

  window_destroy(s_window);
  s_window = NULL;
  window_stack_pop_all(true);  // Don't show splash on exit
}

void transit_system_selection_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
#endif