#include "list_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_status_bar;

static GBitmap *s_icon_bitmap;

/********************************* MenuLayer **********************************/

void draw_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  GRect bounds = layer_get_bounds(cell_layer);
  int index = cell_index->row;

  HotelData *hotel_data = data_get_hotel(index);

  // Background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, GCornerNone, 0);

  // Z icon
  GRect icon_rect = GRect(
    bounds.origin.x + 5,
    bounds.origin.y + 8,
    25,
    25
  );
  graphics_draw_bitmap_in_rect(ctx, s_icon_bitmap, icon_rect);

  // Hotel name
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    hotel_data->name,
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GRect(
      bounds.origin.x + 35,
      bounds.origin.y - 5,
      bounds.size.w - 40,
      20
    ),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Summary on two lines
  graphics_draw_text(
    ctx,
    hotel_data->summary,
    fonts_get_system_font(FONT_KEY_GOTHIC_14),
    GRect(
      bounds.origin.x + 35,
      bounds.origin.y + 12,
      bounds.size.w - 40,
      50
    ),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Show selected
  if (menu_layer_is_index_selected(s_menu_layer, cell_index)) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, 5, bounds.size.h), GCornerNone, 0);
  }

  // Sep
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.size.h - 2, bounds.size.w, 2), GCornerNone, 0);
}

uint16_t get_num_rows_handler(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return data_get_progress_max();
}

int16_t get_cell_height_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return 45;
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_APP_ICON);

  GEdgeInsets menu_insets = (GEdgeInsets) {.top = STATUS_BAR_LAYER_HEIGHT};
  GRect menu_bounds = grect_inset(bounds, menu_insets);

  s_status_bar = text_layer_create(GRect(0, -3, bounds.size.w, STATUS_BAR_LAYER_HEIGHT + 3));
  text_layer_set_text_color(s_status_bar, GColorWhite);
  text_layer_set_background_color(s_status_bar, GColorBlack);
  text_layer_set_font(s_status_bar, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_status_bar, GTextAlignmentCenter);

  static char status_text[32];
  snprintf(status_text, sizeof(status_text), "Updated at: %s", data_get_updated_at());
  text_layer_set_text(s_status_bar, status_text);
  layer_add_child(window_layer, text_layer_get_layer(s_status_bar));

  s_menu_layer = menu_layer_create(menu_bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  menu_layer_pad_bottom_enable(s_menu_layer, false);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .draw_row = draw_row_handler,
    .get_num_rows = get_num_rows_handler,
    .get_cell_height = get_cell_height_handler,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  gbitmap_destroy(s_icon_bitmap);
  text_layer_destroy(s_status_bar);

  window_destroy(s_window);
  s_window = NULL;
  window_stack_pop_all(true);  // Don't show splash on exit
}

void list_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}
