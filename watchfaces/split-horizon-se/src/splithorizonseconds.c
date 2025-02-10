#include <pebble.h>

#include "util/util.h"
#include "compat/InverterLayerCompat.h"

//Elements
static Window *s_main_window;
static TextLayer *lower_layer, *time_layer, *am_pm_layer, *day_layer, *date_layer, *month_layer;
#ifdef PBL_SDK_2
static InverterLayer *top_shade, *bottom_shade, *q1_shade, *q2_shade, *q3_shade, *q4_shade;
#elif PBL_SDK_3
static InverterLayerCompat *top_shade, *bottom_shade, *q1_shade, *q2_shade, *q3_shade, *q4_shade;
#endif

//State
static bool s_q_states[4];

static GColor fg_color, bg_color;

/******************************** Time functions **********************************/

static void set_time(struct tm *t) {
  int day = t->tm_mday;
  int seconds = t->tm_sec;
  int hours = t->tm_hour;

  //Time string - static keeps it around
  static char s_hour_text[] = "00:00";
  strftime(s_hour_text, sizeof(s_hour_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  text_layer_set_text(time_layer, s_hour_text);
  
  //AM/PM
  if(!clock_is_24h_style()) {
    //Show AM/PM marker
    layer_set_frame(text_layer_get_layer(am_pm_layer), GRect(113, 35, 40, 40));
    text_layer_set_text(am_pm_layer, (hours < 12) ? "AM" : "PM");
  } else {
    //Hide it
    layer_set_frame(text_layer_get_layer(am_pm_layer), GRect(0, 0, 0, 0));
  }
    
  //Day string
  static char s_day_text[] = "Wed";
  strftime(s_day_text, sizeof(s_day_text), "%a", t);
  text_layer_set_text(day_layer, s_day_text);
  
  //Date string
  static char s_date_text[] = "xxxx";
  strftime(s_date_text, sizeof(s_date_text), "%eth", t);
  if(day == 1 || day == 21 || day == 31) {  
    //1st, 21st, 31st
    s_date_text[2] = 's';
    s_date_text[3] = 't';
  } else if (day == 2 || day == 22) { 
    //2nd, 22nd
    s_date_text[2] = 'n';
    s_date_text[3] = 'd';
  } else if(day == 3 || day == 23) {  
    //3rd, 23rd
    s_date_text[2] = 'r';
    s_date_text[3] = 'd';
  }
  text_layer_set_text(date_layer, s_date_text);

  //Month string
  static char s_month_text[] = "September";
  strftime(s_month_text, sizeof(s_month_text), "%B", t);
  text_layer_set_text(month_layer, s_month_text);
  
  //Synchronise q markers
  if(!s_q_states[0] && seconds >= 15) {
    util_animate_layer(inverter_layer_compat_func_get(q1_shade), GRect(0,0,35,0), GRect(0,0,35,10), 1000, 0);
    s_q_states[0] = true;
  }

  if(!s_q_states[1] && seconds >= 30) {
    util_animate_layer(inverter_layer_compat_func_get(q2_shade), GRect(36,0,35,0), GRect(36,0,35,10), 1000, 0);
    s_q_states[1] = true;
  }

  if(!s_q_states[2] && seconds >= 45) {
    util_animate_layer(inverter_layer_compat_func_get(q3_shade), GRect(72,0,35,0), GRect(72,0,35,10), 1000, 0);
    s_q_states[2] = true;
  }
}

static void tick_handler(struct tm *t, TimeUnits units_changed) {
  //Get seconds
  int seconds = t->tm_sec;

  //Animate markers and shades
  switch(seconds) {
    case 0:
      //Change time
      set_time(t);
      break;
    case 1:
      //Animate shades out
      util_animate_layer(inverter_layer_compat_func_get(top_shade), GRect(0,0,144,84), GRect(0,0,144,0), 400, 0);
      util_animate_layer(inverter_layer_compat_func_get(bottom_shade), GRect(0,84,144,84), GRect(0,168,144,0), 400, 0);
      
      //Retract q markers
      util_animate_layer(inverter_layer_compat_func_get(q1_shade), GRect(0,0,35,10), GRect(0,0,35,0), 300, 0);
      util_animate_layer(inverter_layer_compat_func_get(q2_shade), GRect(36,0,35,10), GRect(36,0,35,0), 300, 0);
      util_animate_layer(inverter_layer_compat_func_get(q3_shade), GRect(72,0,35,10), GRect(72,0,35,0), 300, 0);
      util_animate_layer(inverter_layer_compat_func_get(q4_shade), GRect(108,0,35,10), GRect(108,0,35,0), 300, 0);
      s_q_states[0] = false; 
      s_q_states[1] = false; 
      s_q_states[2] = false; 
      s_q_states[3] = false;
      break;
    case 15: 
      util_animate_layer(inverter_layer_compat_func_get(q1_shade), GRect(0,0,35,0), GRect(0,0,35,10), 1000, 0);
      s_q_states[0] = true;
      break;
    case 30:
      util_animate_layer(inverter_layer_compat_func_get(q2_shade), GRect(36,0,35,0), GRect(36,0,35,10), 1000, 0);
      s_q_states[1] = true;
      break;
    case 45:
      util_animate_layer(inverter_layer_compat_func_get(q3_shade), GRect(72,0,35,0), GRect(72,0,35,10), 1000, 0);
      s_q_states[2] = true;
      break;
    case 59: 
      util_animate_layer(inverter_layer_compat_func_get(q4_shade), GRect(108,0,35,0), GRect(108,0,35,10), 1000, 0);
      s_q_states[3] = true;  
      
      //Animate shades in
      util_animate_layer(inverter_layer_compat_func_get(top_shade), GRect(0,0,144,0), GRect(0,0,144,84), 400, 0);
      util_animate_layer(inverter_layer_compat_func_get(bottom_shade), GRect(0,168,144,0), GRect(0,84,144,84), 400, 0);
      break;
  }
}

/******************************** Window functions *******************************/

static void main_window_load(Window *window) {
  //Get resources
  ResHandle font_20 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_20);
  ResHandle font_25 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_25);
  ResHandle font_43 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_43);

  //Bottom frame
  lower_layer = text_layer_create(GRect(0, 84, 144, 84));
  text_layer_set_background_color(lower_layer, fg_color);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(lower_layer));

  //TextLayers
  time_layer = util_init_text_layer(GRect(1, 41, 150, 50), fg_color, GColorClear, font_43, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  am_pm_layer = util_init_text_layer(GRect(0, 0, 0, 0), fg_color, GColorClear, font_20, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(am_pm_layer));

  day_layer = util_init_text_layer(GRect(1, 75, 150, 50), bg_color, GColorClear, font_25, GTextAlignmentLeft);
  text_layer_set_text(day_layer, "MON");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_layer));

  date_layer = util_init_text_layer(GRect(70, 75, 150, 50), bg_color, GColorClear, font_25, GTextAlignmentLeft);
  text_layer_set_text(date_layer, "25");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

  month_layer = util_init_text_layer(GRect(1, 95, 150, 50), bg_color, GColorClear, font_20, GTextAlignmentLeft);
  text_layer_set_text(month_layer, "JANUARY");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(month_layer));

  //InverterLayers
  top_shade = inverter_layer_compat_func_create(GRect(0, 0, 144, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_compat_func_get(top_shade));

  bottom_shade = inverter_layer_compat_func_create(GRect(0, 0, 144, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_compat_func_get(bottom_shade));

  q1_shade = inverter_layer_compat_func_create(GRect(0, 0, 35, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_compat_func_get(q1_shade));

  q2_shade = inverter_layer_compat_func_create(GRect(36, 0, 35, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_compat_func_get(q2_shade));

  q3_shade = inverter_layer_compat_func_create(GRect(72, 0, 35, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_compat_func_get(q3_shade));

  q4_shade = inverter_layer_compat_func_create(GRect(108, 0, 35, 0));
  layer_add_child(window_get_root_layer(window), inverter_layer_compat_func_get(q4_shade));

  //Init display not blank
  time_t temp = time(NULL);  
  struct tm *t = localtime(&temp);
  set_time(t);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(lower_layer);
  text_layer_destroy(time_layer);
  text_layer_destroy(am_pm_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(month_layer);

  inverter_layer_compat_func_destroy(top_shade);
  inverter_layer_compat_func_destroy(bottom_shade);
  inverter_layer_compat_func_destroy(q1_shade);
  inverter_layer_compat_func_destroy(q2_shade);
  inverter_layer_compat_func_destroy(q3_shade);
  inverter_layer_compat_func_destroy(q4_shade);

  window_destroy(s_main_window);
}

static void init() {
  s_main_window = window_create();
  WindowHandlers handlers = {
    .load = main_window_load,
    .unload = main_window_unload
  };
  window_set_window_handlers(s_main_window, handlers);

  fg_color = COLOR_FALLBACK(GColorRed, GColorBlack);
  bg_color = GColorWhite;
#ifdef PBL_SDK_3
  inverter_layer_compat_set_colors(bg_color, fg_color);
#endif

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  window_stack_push(s_main_window, true);
}

int main() {
  init();
  app_event_loop();

  return 0;
}
