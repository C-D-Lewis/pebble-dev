#include <pebble.h>
#include "global.h"

//Splash Elements
static Window* splash_window;
static TextLayer *progress_layer;
static BitmapLayer *logo_layer;
static GBitmap *logo_gbitmap;

//State
static int 
  current_title_index = 0,
  current_description_index = 0;

/**************************** Networking *********************************/

static void in_recv_handler(DictionaryIterator *iter, void *context)
{
  Tuple *t = dict_read_first(iter);
  int key = t->key;

  while(t != NULL)
  {
    //Must be processed first!
    if(key == KEY_QUANTITY)
    {
      max_quantity = t->value->int32;
      app_log(APP_LOG_LEVEL_INFO, "Pebble", 0, "max_quantity=%d", max_quantity);

      //Show titles Window for streaming
      title_window_push();
    }

    //Configuration
    if(key == KEY_DETAIL_SIZE)
    {
      if(strcmp("18", t->value->cstring) == 0)
      {
        //Use font size 18
        persist_write_int(KEY_DETAIL_SIZE, 18);

        app_log(APP_LOG_LEVEL_INFO, "BBC News", 0, "Chose font size 18");
      }
      else if(strcmp("24", t->value->cstring) == 0)
      {
        persist_write_int(KEY_DETAIL_SIZE, 24);

        app_log(APP_LOG_LEVEL_INFO, "BBC News", 0, "Chose font size 24");
      }
      else
      {
        app_log(APP_LOG_LEVEL_INFO, "BBC News", 0, "Chose OTHER font size '%s'", t->value->cstring);
      }
    }

    //Handle data incoming
    else if(current_title_index < max_quantity || current_description_index < max_quantity)
    {
      switch(key)
      {

      //A title
      case KEY_TITLE:
        titles[current_title_index] = malloc((strlen(t->value->cstring) * sizeof(char)) + 1);
        snprintf(titles[current_title_index], (strlen(t->value->cstring) * sizeof(char)) + 1, "%s", t->value->cstring);
        
        current_title_index++;
        break;

      //A description
      case KEY_DESCRIPTION:
        descriptions[current_description_index] = malloc((strlen(t->value->cstring) * sizeof(char)) + 1);
        snprintf(descriptions[current_description_index], (strlen(t->value->cstring) * sizeof(char)) + 1, "%s", t->value->cstring);
        
        current_description_index++;
        break;
      }

      //Update UI to show progress IF A PAIR IS COMPLETE
      if(current_title_index == current_description_index)
      {
        if(current_quantity < max_quantity)
        {
          current_quantity++;
          menu_layer_reload_data(titles_menu_layer);
        }
      }
    }

    //Ready!
    else
    {
      app_log(APP_LOG_LEVEL_INFO, "Pebble", 0, "All %d stories received!", current_quantity);

      //Save power
      app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
    }

    //Finally
    t = dict_read_next(iter);
  }
}

/************************** Splash Window ***********************************/

static void splash_window_load(Window *window)
{
  //Logo
  logo_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);
  logo_layer = bitmap_layer_create(GRect(10, 25, 122, 50));
  bitmap_layer_set_bitmap(logo_layer, logo_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(logo_layer));

  //Text output
  progress_layer = cl_text_layer_create(GRect(10, 90, 122, 24), GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_18, GTextAlignmentCenter);
  text_layer_set_text(progress_layer, "Waiting...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(progress_layer));
}
 
static void splash_window_unload(Window *window)
{
  //Destroy Layers
  bitmap_layer_destroy(logo_layer);
  gbitmap_destroy(logo_gbitmap);
  text_layer_destroy(progress_layer);

  //Self destroy Window
  window_destroy(window);
}

/********************************* App *************************************/
 
static void init()
{
  //Show AppMessage errors
  cl_set_debug(true);

  //Show initial Window
  splash_window = window_create();
  WindowHandlers handlers = {
    .load = splash_window_load,
    .unload = splash_window_unload
  };
  window_set_window_handlers(splash_window, (WindowHandlers) handlers);

  //Open AppMessage at high speed
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  cl_app_message_open(
    app_message_inbox_size_maximum(),
    app_message_outbox_size_maximum(),
    (AppMessageInboxReceived)in_recv_handler
  );

  //Show Window
  window_stack_push(splash_window, true);
}
 
static void deinit()
{
  //Free content buffers
  for(int i = 0; i < current_quantity; i++)
  {
    if(titles[i] != NULL)
    {
      free(titles[i]);
    }
    if(descriptions[i] != NULL)
    {
      free(descriptions[i]);
    } 
  }

  app_log(APP_LOG_LEVEL_INFO, "Pebble", 0, "deinit() completed.");
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}