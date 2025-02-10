#include <pebble.h>
#include "cl_util.h"

#define DEBUG false

//1.X UUID - 6f8fd9ff-1af9-4e32-8b21-8cd642949f75

static Window *window;
static GBitmap *background_bitmap;
static BitmapLayer *background_layer;
static TextLayer *hours_layer, *minutes_layer, *day_layer, *voltage_layer;
static InverterLayer *electron_layer;

//State
static bool setup = false;
static int x = 21, y = 78, w = 3, h = 3;

static void tick_callback(struct tm *t, TimeUnits unit_changed)
{
  int days = t->tm_mday;
  int hours = t->tm_hour;
  int minutes = t->tm_min;
  int seconds = t->tm_sec;

  //Update resistors and output voltage
  if(seconds % 60 == 0 || setup == false)
  {
    //Day of the month is the battery voltage
    static char day_buff[] = "XXV";
    snprintf(day_buff, sizeof("XXV"), "%dV", days);
    text_layer_set_text(day_layer, day_buff);

    //R1 is the hours
    static char hour_buff[] = "XX";
    if(clock_is_24h_style() == true)
    {
      strftime(hour_buff, sizeof("XX"), "%H", t);
    }
    else
    {
      strftime(hour_buff, sizeof("XX"), "%I", t);
    }
    text_layer_set_text(hours_layer, hour_buff);

    //R2 is the minutes
    static char min_buff[] = "XX";
    strftime(min_buff, sizeof("XX"), "%M", t);
    text_layer_set_text(minutes_layer, min_buff);

    //Output voltage
    float voltage = ((float)minutes/((float)hours + (float)minutes)) * (float)days;

    //Show on layout
    static char voltage_buff[] = "5.000V";
    snprintf(voltage_buff, sizeof("5.000V"), "%dV", (int)voltage);
    text_layer_set_text(voltage_layer, voltage_buff);

    //DEBUG
    if(DEBUG == true)
    {
      text_layer_set_text(day_layer, "33V");
      text_layer_set_text(hours_layer, "24");
      text_layer_set_text(minutes_layer, "58");
      text_layer_set_text(voltage_layer, "28V");
    }
  }

  //Update moving Inverterlayer secondsLayer
  if(seconds == 0 || setup == false)
  {  
    x = 21;
    y = 78;
    setup = true;
  }
  if(seconds < 10)
  {  
    y -= 6;
    if(x != 21)
    {
      x = 21;
    }
  }
  if(seconds == 10)
  {
    y = 16;
  }
  if(seconds > 10 && seconds < 20) 
  {  
    x += 7;
    if(y != 16)
    {
      y = 16;
    }
  }
  if(seconds == 20)
  {
    x = 94;
  }
  if(seconds > 20 && seconds < 40) 
  {  
    y += 7;
    if(x != 94)
    {
      x = 94;
    }
  }
  if(seconds == 40)
  {
    y = 151;
  }
  if(seconds > 40 && seconds < 51) 
  {  
    x -= 7;
    if(y != 151)
    {
     y = 151;
    }
  }
  if(seconds == 51)
  {
    x = 21;
  }
  if(seconds > 51 && seconds < 60) 
  {   
    y -= 7;
    if(x != 21)
    {
      x = 21;
    }
  }

  //Draw moving electron
  cl_animate_layer(inverter_layer_get_layer(electron_layer), layer_get_frame(inverter_layer_get_layer(electron_layer)), GRect(x, y, w, h), 500, 0);
}

static void window_load(Window *window) 
{
  //Load resources
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);

  //Background
  background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background_layer, background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));

  //Electron
  electron_layer = inverter_layer_create(GRect(0, 0, 0, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(electron_layer));

  //TextLayers
  day_layer = cl_init_text_layer(GRect(38, 65, 60, 30), GColorWhite, GColorClear, false, 0, FONT_KEY_GOTHIC_28, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_layer));

  hours_layer = cl_init_text_layer(GRect(50, 28, 60, 30), GColorWhite, GColorClear, false, 0, FONT_KEY_GOTHIC_28, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hours_layer));

  minutes_layer = cl_init_text_layer(GRect(50, 95, 60, 30), GColorWhite, GColorClear, false, 0, FONT_KEY_GOTHIC_28, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(minutes_layer));

  voltage_layer = cl_init_text_layer(GRect(105, 50, 60, 30), GColorWhite, GColorClear, false, 0, FONT_KEY_GOTHIC_28, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(voltage_layer));
}

static void window_unload(Window *window) 
{
  gbitmap_destroy(background_bitmap);
  bitmap_layer_destroy(background_layer);
  inverter_layer_destroy(electron_layer);
  text_layer_destroy(hours_layer);
  text_layer_destroy(minutes_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(voltage_layer);
}

static void init(void) 
{
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_callback);

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
