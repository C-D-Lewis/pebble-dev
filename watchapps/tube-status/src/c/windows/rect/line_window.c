#if defined(PBL_RECT)
#include "line_window.h"

#define LOGO_RADIUS 13
#define STRIPE_WIDTH 12
#ifdef PBL_PLATFORM_EMERY
  #define LOGO_MARGIN 12
#else
  #define LOGO_MARGIN 10
#endif

static Window *s_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_bar;

bool line_has_reason(int index) {
  char *reason = data_get_line_reason(index);
  return strlen(reason) != 0;
}

/******************************** Click config *********************************/

static void select_click_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  int index = (int)cell_index->row;
  if (!line_has_reason(index)) return;

  reason_window_push(index);
}

/********************************* MenuLayer **********************************/

void draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  GRect bounds = layer_get_bounds(cell_layer);

  // Important that all lines are in the same order as LineType and zero-indexed
  int type = cell_index->row;

#ifdef PBL_PLATFORM_EMERY
  const int logo_bounds_x = bounds.origin.x + (LOGO_MARGIN / 3) + 2;
#else
  const int logo_bounds_x = bounds.origin.x + (LOGO_MARGIN / 3);
#endif
  GRect logo_bounds = GRect(
    logo_bounds_x,
    (bounds.size.h - (2 * LOGO_RADIUS)) / 2,
    (2 * LOGO_RADIUS),
    (2 * LOGO_RADIUS)
  );
  GPoint center = grect_center_point(&logo_bounds);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, GCornerNone, 0);

  // Line color
  int line_color_x = center.x - (STRIPE_WIDTH / 2);
  graphics_context_set_fill_color(ctx, data_get_line_color(type));
  graphics_fill_rect(
    ctx,
    GRect(line_color_x, bounds.origin.y, STRIPE_WIDTH, bounds.size.h),
    GCornerNone,
    0
  );
  if (data_get_line_color_is_striped(type)) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(
      ctx,
      GRect(line_color_x + (STRIPE_WIDTH / 3), bounds.origin.y, STRIPE_WIDTH / 3, bounds.size.h),
      GCornerNone,
      0
    );
  }

  // Draw circle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, LOGO_RADIUS - 1);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, (5 * (LOGO_RADIUS - 1)) / 7);

  // Issue?
  graphics_context_set_fill_color(ctx, data_get_line_state_color(type));
  graphics_fill_circle(ctx, center, (5 * (LOGO_RADIUS - 1)) / 7);

  // Show selected
  if(menu_layer_is_index_selected(s_menu_layer, cell_index)) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, center, (4 * (LOGO_RADIUS - 2)) / 7);
  }

  // Info
  GRect text_bounds = GRect(
    bounds.origin.x + (3 * LOGO_MARGIN) + 2,
    bounds.origin.y - 5,
    bounds.size.w - (3 * LOGO_MARGIN),
    30
  );
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    data_get_line_name(type),
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    text_bounds,
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  text_bounds.origin.y += 25;
  text_bounds.size.w -= LOGO_MARGIN;
  graphics_draw_text(
    ctx,
    data_get_line_state(type),
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    text_bounds,
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Show if reason can be viewed
  if(menu_layer_is_index_selected(s_menu_layer, cell_index) && line_has_reason(cell_index->row)) {
    graphics_draw_text(
      ctx,
      ">",
      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
      GRect(bounds.size.w - 10, 2, 32, 32),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft,
      NULL
    );
  }

  // Sep
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.size.h - 2, bounds.size.w, 2), GCornerNone, 0);
}

uint16_t get_num_rows_handler(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return LineTypeMax;
}

int16_t get_cell_height_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return 45;
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
    .get_cell_height = get_cell_height_handler,
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
