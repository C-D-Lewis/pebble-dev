#include "contact_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

static GBitmap *s_arrows_bitmap;

/********************************* MenuLayer **********************************/

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 4;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  int row = cell_index->row;
  GRect bounds = layer_get_bounds(cell_layer);
  GSize bitmap_size = gbitmap_get_bounds(s_arrows_bitmap).size;

  switch(row) {
    case 0:
#if defined(PBL_COLOR)
      graphics_context_set_text_color(ctx,
        data_get_contact_valid(ContactTypeSMS, cell_index->row) ? GColorWhite : GColorDarkGray);
#endif
      menu_cell_basic_draw(ctx, cell_layer, data_get_contact_name(ContactTypeSMS, 0), "Last SMS", NULL);
      break;
    case 1:
#if defined(PBL_COLOR)
      graphics_context_set_text_color(ctx,
        data_get_contact_valid(ContactTypeCall, cell_index->row) ? GColorWhite : GColorDarkGray);
#endif
      menu_cell_basic_draw(ctx, cell_layer, data_get_contact_name(ContactTypeCall, 0), "Last Call", NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Top Incoming", "Click for more...", NULL);

      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, s_arrows_bitmap,
        GRect(bounds.size.w - (3 * bitmap_size.w), (bounds.size.h - bitmap_size.h) / 2, bitmap_size.w, bitmap_size.h));
      break;
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "Favorites", "Click for more...", NULL);

      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, s_arrows_bitmap,
        GRect(bounds.size.w - (3 * bitmap_size.w), (bounds.size.h - bitmap_size.h) / 2, bitmap_size.w, bitmap_size.h));
      break;
    default:
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT,
    44);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      if(data_get_contact_valid(ContactTypeSMS, 0)) {
        responses_window_push(ContactTypeSMS, 0);
      }
      break;
    case 1:
      if(data_get_contact_valid(ContactTypeCall, 0)) {
        responses_window_push(ContactTypeCall, 0);
      }
      break;
    case 2:
      sub_window_push(ContactTypeIncoming);
      break;
    case 3:
      sub_window_push(ContactTypeFavorite);
      break;
    default:
      break;
  }
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_arrows_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ARROWS);

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

  gbitmap_destroy(s_arrows_bitmap);

  window_destroy(s_window);
  s_window = NULL;

  window_stack_pop_all(false);
}

/************************************ API *************************************/

void contact_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);
}

void contact_window_reload_data() {
  menu_layer_reload_data(s_menu_layer);
}
