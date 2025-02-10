#include <pebble.h>
#include "cl_util.h"

#define GRECT_PHASE_HIDDEN GRect(-144, 59, 144, 51)
#define GRECT_PHASE_SHOWING GRect(0, 59, 144, 51)

#define GRECT_TIME_BG_SHOWING GRect(40, 53, 63, 60)
#define GRECT_TIME_BG_HIDDEN GRect(40, 168, 63, 60)
#define GRECT_TIME_SHOWING GRect(40, 68, 63, 24)
#define GRECT_TIME_HIDDEN GRect(40, 163, 63, 24)

#define GRECT_BATTERY_SHOWING GRect(45, 95, 50, 20)
#define GRECT_BATTERY_HIDDEN GRect(50, 168, 55, 20)

#define GRECT_DATE_BG_SHOWING GRect(27, 128, 90, 24)
#define GRECT_DATE_BG_HIDDEN GRect(27, 168, 90, 24)
#define GRECT_DATE_SHOWING GRect(27, 123, 90, 30)

#define GRECT_BT_SHOWING GRect(62, 58, 20, 15)
#define GRECT_BT_HIDDEN GRect(62, 168, 20, 15)

//Layout elements
static Window *window;
static BitmapLayer 
  *background_white_layer, 
  *background_black_layer,
  *bt_layer;
static GBitmap 
  *background_white_bitmap, 
  *background_black_bitmap,
  *power_bitmap,
  *bt_on_bitmap,
  *bt_off_bitmap;
static Layer 
  *phase_layer,
  *time_bg_layer,
  *battery_layer,
  *date_bg_layer;
static TextLayer 
  *time_layer,
  *date_layer;

//State
static AppTimer *stat_timer;
static bool triggd = false;

static void phase_update_proc(Layer *layer, GContext *ctx)
{
  //Get which hour
  time_t temp = time(NULL); 
  struct tm *t = localtime(&temp); 
  int hours = t->tm_hour;
  int minutes = t->tm_min;

  if(hours > 0)
  {
    graphics_context_set_fill_color(ctx, GColorWhite);

    int x = 19;
    x += ((hours + 1) * 4);
    x += minutes / 15;

    graphics_fill_circle(ctx, GPoint(x, 25), 25);
  }
}

static void bg_update_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_stroke_color(ctx, GColorWhite);
  GRect bounds = layer_get_bounds(layer);

  //3 layer filled round rect
  cl_fill_chamfer_rect(ctx, 4, bounds.size.w, bounds.size.h);
}

static void battery_update_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(20, 2, 24, 10), 1, GCornersAll);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  BatteryChargeState state = battery_state_service_peek();
  uint8_t perc = state.charge_percent;

  graphics_fill_rect(ctx, GRect(22, 4, 2 * (perc / 10), 6), 1, GCornersAll);

  graphics_draw_bitmap_in_rect(ctx, power_bitmap, GRect(8, 0, 8, 14));
}

static void window_load(Window *window) 
{
  window_set_background_color(window, GColorBlack);

  //Power bitmap
  power_bitmap = gbitmap_create_with_resource(RESOURCE_ID_POWER);

  //Phase
  phase_layer = layer_create(GRECT_PHASE_SHOWING);
  layer_set_update_proc(phase_layer, (LayerUpdateProc)phase_update_proc);
  layer_add_child(window_get_root_layer(window), phase_layer);

  //Time display under background
  time_bg_layer = layer_create(GRECT_TIME_BG_HIDDEN);
  time_layer = cl_init_text_layer(GRECT_TIME_BG_HIDDEN, GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  layer_set_update_proc(time_bg_layer, (LayerUpdateProc)bg_update_proc);
  layer_add_child(window_get_root_layer(window), time_bg_layer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  //Bluetooth
  bt_on_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ON);
  bt_off_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_OFF);
  bt_layer = bitmap_layer_create(GRECT_BT_HIDDEN);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bt_layer));

  //Battery
  battery_layer = layer_create(GRECT_BATTERY_HIDDEN);
  layer_set_update_proc(battery_layer, (LayerUpdateProc)battery_update_proc);
  layer_add_child(window_get_root_layer(window), battery_layer);

  //Background
  background_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_WHITE);
  background_white_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(background_white_layer, GCompOpOr);
  bitmap_layer_set_bitmap(background_white_layer, background_white_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_white_layer));

  background_black_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_BLACK);
  background_black_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(background_black_layer, GCompOpAnd);
  bitmap_layer_set_bitmap(background_black_layer, background_black_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_black_layer)); 

  //Date
  date_bg_layer = layer_create(GRECT_DATE_BG_HIDDEN);
  date_layer = cl_init_text_layer(GRECT_DATE_BG_HIDDEN, GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  layer_set_update_proc(date_bg_layer, (LayerUpdateProc)bg_update_proc);
  layer_add_child(window_get_root_layer(window), date_bg_layer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  //Animate in
  cl_animate_layer(phase_layer, GRECT_PHASE_HIDDEN, GRECT_PHASE_SHOWING, 1000, 200);
}

static void window_unload(Window *window) 
{
  //Background
  gbitmap_destroy(background_black_bitmap);
  gbitmap_destroy(background_white_bitmap);
  bitmap_layer_destroy(background_black_layer);
  bitmap_layer_destroy(background_white_layer);

  //Phase
  layer_destroy(phase_layer);

  //Time
  layer_destroy(time_bg_layer);
  text_layer_destroy(time_layer);

  //Battery
  gbitmap_destroy(power_bitmap);
  layer_destroy(battery_layer);

  //Date
  layer_destroy(date_bg_layer);
  text_layer_destroy(date_layer);

  //Bluetooth
  bitmap_layer_destroy(bt_layer);
  gbitmap_destroy(bt_on_bitmap);
  gbitmap_destroy(bt_off_bitmap);
}

static void stat_timer_callback(void *data)
{
  //Show phase
  cl_animate_layer(phase_layer, GRECT_PHASE_HIDDEN, GRECT_PHASE_SHOWING, 1000, 0);

  //Hide time
  cl_animate_layer(time_bg_layer, GRECT_TIME_BG_SHOWING, GRECT_TIME_BG_HIDDEN, 500, 0);
  cl_animate_layer(text_layer_get_layer(time_layer), GRECT_TIME_SHOWING, GRECT_TIME_HIDDEN, 500, 0);

  //Hide battery
  cl_animate_layer(battery_layer, GRECT_BATTERY_SHOWING, GRECT_BATTERY_HIDDEN, 500, 0);

  //Hide date
  cl_animate_layer(date_bg_layer, GRECT_DATE_BG_SHOWING, GRECT_DATE_BG_HIDDEN, 500, 0);
  cl_animate_layer(text_layer_get_layer(date_layer), GRECT_DATE_SHOWING, GRECT_DATE_BG_HIDDEN, 500, 0);

  cl_animate_layer(bitmap_layer_get_layer(bt_layer), GRECT_BT_SHOWING, GRECT_BT_HIDDEN, 500, 0);

  triggd = false;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) 
{
  if(triggd == false)
  {
    //Get bluetooth state
    bool connected = bluetooth_connection_service_peek();
    if(connected == true)
    {
      bitmap_layer_set_bitmap(bt_layer, bt_on_bitmap);
    }
    else
    {
      bitmap_layer_set_bitmap(bt_layer, bt_off_bitmap);
    }

    //Hide phase
    cl_animate_layer(phase_layer, layer_get_frame(phase_layer), GRect(144, 59, 144, 51), 500, 100);

    //Show time
    cl_animate_layer(time_bg_layer, GRECT_TIME_BG_HIDDEN, GRECT_TIME_BG_SHOWING, 500, 100);
    cl_animate_layer(text_layer_get_layer(time_layer), GRECT_TIME_HIDDEN, GRECT_TIME_SHOWING, 500, 100);

    //Show battery
    cl_animate_layer(battery_layer, GRECT_BATTERY_HIDDEN, GRECT_BATTERY_SHOWING, 500, 100);

    //Show date
    cl_animate_layer(date_bg_layer, GRECT_DATE_BG_HIDDEN, GRECT_DATE_BG_SHOWING, 500, 100);
    cl_animate_layer(text_layer_get_layer(date_layer), GRECT_DATE_BG_HIDDEN, GRECT_DATE_SHOWING, 500, 100);

    //Animate bluetooth layer
    cl_animate_layer(bitmap_layer_get_layer(bt_layer), GRECT_BT_HIDDEN, GRECT_BT_SHOWING, 500, 100);

    //Prep dissapear
    stat_timer = app_timer_register(5000, (AppTimerCallback)stat_timer_callback, NULL);

    triggd = true;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  static char buff[8];
  if(clock_is_24h_style())
  {
    strftime(buff, 8 * sizeof(char), "%H:%M", tick_time);
  }
  else
  {
    strftime(buff, 8 * sizeof(char), "%I:%M", tick_time);
  }
  text_layer_set_text(time_layer, buff);

  static char date_buff[16];
  strftime(date_buff, 16 * sizeof(char), "%a %d %b", tick_time);
  text_layer_set_text(date_layer, date_buff);
}

static void init(void) 
{
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  accel_tap_service_subscribe(&accel_tap_handler);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window_stack_push(window, true);
}

static void deinit(void) 
{
  accel_tap_service_unsubscribe();

  tick_timer_service_unsubscribe();

  window_destroy(window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}
