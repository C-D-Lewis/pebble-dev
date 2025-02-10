#include <pebble.h>

//Layers
static Window *window;
static TextLayer *time_label, *time_layer, *date_label, *date_layer, *prompt_label;
static InverterLayer *prompt_layer;
static AppTimer *timer;

//Buffers
static char time_buffer[] = "XX:XX", date_buffer[] = "XX/XX/XXXX";

//State
static bool flash_visible = false;

//Prototypes
static TextLayer* cl_init_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment);

/****************************** Time Lifecycle **********************************/

static void set_time(struct tm *t)
{
  //Set time
  if(clock_is_24h_style())
  {
    strftime(time_buffer, sizeof("XX:XX"),"%H:%M", t);
  }
  else
  {
    strftime(time_buffer, sizeof("XX:XX"),"%I:%M", t);
  }
  text_layer_set_text(time_layer, time_buffer);

  //Set date
  strftime(date_buffer, sizeof("XX/XX/XXXX"), "%d/%m/%Y", t);
  text_layer_set_text(date_layer, date_buffer);
}

static void tick_handler(struct tm *t, TimeUnits units_changed)
{
  //Change time display
  set_time(t);

  //Do prompt flash
  if(flash_visible)
  {
    flash_visible = false;
    layer_remove_from_parent(inverter_layer_get_layer(prompt_layer));
  }
  else
  {
    flash_visible = true;
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(prompt_layer));
  }

}

/***************************** Window Lifecycle *********************************/

static void window_load(Window *window) {
  //Font
  ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_LUCIDA_16);

  //Time 
  time_label = cl_init_text_layer(GRect(5, 5, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(time_label, "C:\\>time /t");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_label));

  time_layer = cl_init_text_layer(GRect(5, 24, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(time_layer, "NA:NA");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  //Date
  date_label = cl_init_text_layer(GRect(5, 53, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(date_label, "C:\\>date /t");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_label));

  date_layer = cl_init_text_layer(GRect(5, 70, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(date_layer, "XX/XX/XXXX");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  //Prompt
  prompt_label = cl_init_text_layer(GRect(5, 99, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(prompt_label, "C:\\>");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(prompt_label));

  prompt_layer = inverter_layer_create(GRect(45, 115, 12, 2));

  //Initial time display
  time_t temp = time(NULL);    
  struct tm *t = localtime(&temp);
  set_time(t);
}

static void window_unload(Window *window) {
  //Time
  text_layer_destroy(time_label);
  text_layer_destroy(time_layer);

  //Date
  text_layer_destroy(date_label);
  text_layer_destroy(date_layer);

  //Prompt
  text_layer_destroy(prompt_label);
  inverter_layer_destroy(prompt_layer);
}

/******************************** App Lifecycle *********************************/

static void init(void) {
  window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, handlers);
  window_set_background_color(window, GColorBlack);

  //Get tick events
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  //Finally
  window_stack_push(window, true);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();

  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

/****************************** Other functions *********************************/

static TextLayer* cl_init_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_load_custom_font(handle));
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}
