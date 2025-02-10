#include <pebble.h>

#define KEY_INVERT 0

static Window *window;
static TextLayer *text_layer;

static void window_load(Window *window) 
{
  //Check for saved option
  bool inverted = persist_read_bool(KEY_INVERT);

  //Create TextLayer
  text_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  //Option-specific setup
  if(inverted == true)
  {
    text_layer_set_text_color(text_layer, GColorWhite);
    text_layer_set_background_color(text_layer, GColorBlack);
    text_layer_set_text(text_layer, "Inverted!");
  }
  else
  {
    text_layer_set_text_color(text_layer, GColorBlack);
    text_layer_set_background_color(text_layer, GColorWhite);
    text_layer_set_text(text_layer, "Not inverted!");
  }

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) 
{
  text_layer_destroy(text_layer);
}

static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
  //Get Tuple
  Tuple *t = dict_read_first(iterator);
  if(t)
  {
    switch(t->key)
    {
    case KEY_INVERT:
      //It's the KEY_INVERT key
      if(strcmp(t->value->cstring, "on") == 0)
      {
        //Set and save as inverted
        text_layer_set_text_color(text_layer, GColorWhite);
        text_layer_set_background_color(text_layer, GColorBlack);
        text_layer_set_text(text_layer, "Inverted!");

        persist_write_bool(KEY_INVERT, true);
      }
      else if(strcmp(t->value->cstring, "off") == 0)
      {
        //Set and save as not inverted
        text_layer_set_text_color(text_layer, GColorBlack);
        text_layer_set_background_color(text_layer, GColorWhite);
        text_layer_set_text(text_layer, "Not inverted!");

        persist_write_bool(KEY_INVERT, false);
      }
      break;
    }
  }
}

static void init(void) 
{
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window_stack_push(window, true);
}

static void deinit(void) 
{
  window_destroy(window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}
