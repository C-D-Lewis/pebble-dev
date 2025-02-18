#include "global.h"

//Detail Elements
static Window *detail_window;
static TextLayer *title_layer, *description_layer;
static ScrollLayer *scroll_layer;

//State
static bool window_created = false;

/************************** Window ********************************/

static void detail_window_load(Window *window)
{
  title_layer = cl_text_layer_create(GRect(5, 0, 134, 100), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentLeft);
  text_layer_set_overflow_mode(title_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(title_layer, titles[current_item]);
  GSize title_max_size = text_layer_get_content_size(title_layer);
  text_layer_set_size(title_layer, GSize(134, title_max_size.h + 14));

  //Respect detail font size choice
  int choice = persist_read_int(KEY_DETAIL_SIZE);
  app_log(APP_LOG_LEVEL_INFO, "BBC News", 0, "Read font size: %d", choice);
  if(choice == 24)
  {
    description_layer = cl_text_layer_create(GRect(5, title_max_size.h + 5, 134, 2000), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24, GTextAlignmentLeft);
  }
  else
  {
    description_layer = cl_text_layer_create(GRect(5, title_max_size.h + 5, 134, 2000), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  }

  text_layer_set_overflow_mode(description_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(description_layer, descriptions[current_item]);
  GSize desc_max_size = text_layer_get_content_size(description_layer);
  text_layer_set_size(description_layer, GSize(134, desc_max_size.h + 14));

  scroll_layer = scroll_layer_create(GRect(0, 0, 144, 168 - CHROME_HEIGHT));
  scroll_layer_set_click_config_onto_window(scroll_layer, window);
  scroll_layer_set_content_size(scroll_layer, GSize(134, title_max_size.h + desc_max_size.h + 14));
  scroll_layer_add_child(scroll_layer, text_layer_get_layer(title_layer));
  scroll_layer_add_child(scroll_layer, text_layer_get_layer(description_layer));

  layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(scroll_layer));

  window_created = true;
}
 
static void detail_window_unload(Window *window)
{
  text_layer_destroy(title_layer);
  text_layer_destroy(description_layer);
  scroll_layer_destroy(scroll_layer);

  window_destroy(window);

  window_created = false;
}

void detail_window_push()
{
  //Created?
  if(window_created == false)
  {
    detail_window = window_create();
    WindowHandlers handlers = {
      .load = detail_window_load,
      .unload = detail_window_unload
    };
    window_set_window_handlers(detail_window, (WindowHandlers) handlers);
    window_stack_push(detail_window, true);
  }

  //Re-show
  else
  {
    //Update Layers - title
    text_layer_set_text(title_layer, titles[current_item]);
    GSize title_max_size = text_layer_get_content_size(title_layer);
    text_layer_set_size(title_layer, GSize(134, title_max_size.h + 14));

    //description
    text_layer_set_text(description_layer, descriptions[current_item]);
    GSize desc_max_size = text_layer_get_content_size(description_layer);
    text_layer_set_size(description_layer, GSize(134, desc_max_size.h + 14));

    //scroll_layer
    scroll_layer_set_content_size(scroll_layer, GSize(134, title_max_size.h + desc_max_size.h + 14));
  }
}