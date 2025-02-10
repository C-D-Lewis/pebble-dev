#include <pebble.h>

static Window *window;
static TextLayer *time_layer;
static GBitmap *kitty_bitmap;
static BitmapLayer *kitty_layer;

static void tick_handler(struct tm *tick_time, TimeUnits changed)
{
  static char buffer[8];

  if(clock_is_24h_style()) 
  {
    strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
  }
  else
  {
    strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
  }
  
  text_layer_set_text(time_layer, buffer);
}

static void window_load(Window *window) 
{
  kitty_layer = bitmap_layer_create(GRect(27, 50, 90, 102));
  bitmap_layer_set_bitmap(kitty_layer, kitty_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(kitty_layer));

  time_layer = text_layer_create(GRect(0, 0, 144, 50));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SILOM_BOL_40)));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
}

static void window_unload(Window *window) 
{
  bitmap_layer_destroy(kitty_layer);
  text_layer_destroy(time_layer);
}

static void init(void) 
{
  kitty_bitmap = gbitmap_create_with_resource(RESOURCE_ID_KITTY);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) 
{
  gbitmap_destroy(kitty_bitmap);
  window_destroy(window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}
