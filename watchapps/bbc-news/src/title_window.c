#include "global.h"

//Titles Elements
static Window *titles_window;
static TextLayer *underlay_layer;

//State
static bool window_created = false;

/********************** Titles MenuLayer **********************************/

static void titles_draw_header_handler(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context)
{
  static char header_buff[32];
  snprintf(header_buff, sizeof("Showing XXX stories"), "Showing %d stories", current_quantity);
  menu_cell_basic_header_draw(ctx, cell_layer, header_buff);
}

static void titles_draw_row_handler(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
  GRect bounds = layer_get_bounds(cell_layer);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    titles[cell_index->row],
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    GRect(bounds.origin.x + 2, bounds.origin.y, bounds.size.w - 4, bounds.size.h),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
}

static uint16_t titles_get_num_rows_handler(MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
  if(current_quantity > 0)
  {
    //TextLayer no longer needed
    layer_remove_from_parent(text_layer_get_layer(underlay_layer));
  }

  return current_quantity;
}

static int16_t titles_get_header_height_handler(MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
  return CHROME_HEIGHT;
}

static void titles_select_click_handler(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
  //Set which item to show in new window
  current_item = cell_index->row;

  detail_window_push();
}

/************************** Titles Window ***********************************/

static void titles_window_load(Window *window)
{
  underlay_layer = cl_text_layer_create(GRect(5, 0, 144, 30), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentCenter);
  text_layer_set_text(underlay_layer, "Loading items...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(underlay_layer));

  titles_menu_layer = menu_layer_create(GRect(0, 0, 144, 168 - CHROME_HEIGHT));
  menu_layer_set_click_config_onto_window(titles_menu_layer, window);
  menu_layer_set_callbacks(titles_menu_layer, NULL, (MenuLayerCallbacks) {
    .draw_row = (MenuLayerDrawRowCallback) titles_draw_row_handler,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) titles_get_num_rows_handler,
    .select_click = (MenuLayerSelectCallback) titles_select_click_handler,                 
    .draw_header = (MenuLayerDrawHeaderCallback) titles_draw_header_handler,                         
    .get_header_height = (MenuLayerGetHeaderHeightCallback) titles_get_header_height_handler         
  });
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(titles_menu_layer));

  window_created = true;
}
 
static void titles_window_unload(Window *window)
{
  text_layer_destroy(underlay_layer);

  menu_layer_destroy(titles_menu_layer);

  window_destroy(window);

  window_created = false;

  //Skip splash
  window_stack_pop_all(false);
}

void title_window_push()
{
	if(window_created == false)
	{
		titles_window = window_create();
    WindowHandlers handlers = {
      .load = titles_window_load,
      .unload = titles_window_unload
    };
    window_set_window_handlers(titles_window, (WindowHandlers) handlers);
    window_stack_push(titles_window, true);
	}

	//Reload
	else
	{
		//Nothing yet!
	}
}