#include <pebble.h>
#include "main_window.h"

#define TYPE_DELTA 200
#define PROMPT_DELTA 1000

static Window *s_window;
static TextLayer *s_time_label, *s_time_layer, *s_date_label, *s_date_layer, *s_prompt_label;
static InverterLayerCompat *s_prompt_layer;
static AppTimer *s_timer;

static char time_buffer[] = "XX:XX", date_buffer[] = "XX/XX/XXXX";
static int state = 0;
static bool prompt_visible = false;

/****************************** Other functions *********************************/

static TextLayer* make_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_load_custom_font(handle));
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}

/****************************** Time Lifecycle **********************************/

static void set_time(struct tm *t) {
  // Set time
  if (clock_is_24h_style()) {
    strftime(time_buffer, sizeof("XX:XX"),"%H:%M", t);
  } else {
    strftime(time_buffer, sizeof("XX:XX"),"%I:%M", t);
  }
  text_layer_set_text(s_time_layer, time_buffer);

  // Set date
  strftime(date_buffer, sizeof("XX/XX/XXXX"), "%d/%m/%Y", t);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void set_time_anim() {
  // Time structures -- Cannot be branch declared
  time_t temp;
  struct tm *t;

  // Do frame animation
  switch (state) {
    case 0:
      // Set time
      temp = time(NULL);
      t = localtime(&temp);
      set_time(t);

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 1:
      text_layer_set_text(s_time_label, "C:\\>t");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 2:
      text_layer_set_text(s_time_label, "C:\\>ti");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 3:
      text_layer_set_text(s_time_label, "C:\\>tim");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 4:
      text_layer_set_text(s_time_label, "C:\\>time");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 5:
      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 6:
      text_layer_set_text(s_time_label, "C:\\>time /");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 7:
      text_layer_set_text(s_time_label, "C:\\>time /t");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 8:
      text_layer_set_text(s_time_label, "C:\\>time /t");
      layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_time_layer));
      text_layer_set_text(s_date_label, "C:\\>");

      s_timer = app_timer_register(10 * TYPE_DELTA, set_time_anim, 0);
      break;
    case 9:
      text_layer_set_text(s_date_label, "C:\\>d");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 10:
      text_layer_set_text(s_date_label, "C:\\>da");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 11:
      text_layer_set_text(s_date_label, "C:\\>dat");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 12:
      text_layer_set_text(s_date_label, "C:\\>date");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 13:
      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 14:
      text_layer_set_text(s_date_label, "C:\\>date /");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 15:
      text_layer_set_text(s_date_label, "C:\\>date /t");

      s_timer = app_timer_register(TYPE_DELTA, set_time_anim, 0);
      break;
    case 16:
      text_layer_set_text(s_date_label, "C:\\>date /t");
      layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_date_layer));
      text_layer_set_text(s_prompt_label, "C:\\>");

      prompt_visible = true;
      s_timer = app_timer_register(PROMPT_DELTA, set_time_anim, 0);
      break;
    default:  // Rest of the minute
      // Do prompt flash
      if (prompt_visible) {
        prompt_visible = false;
        layer_remove_from_parent(inverter_layer_compat_get_layer(s_prompt_layer));
      } else {
        prompt_visible = true;
        layer_add_child(window_get_root_layer(s_window), inverter_layer_compat_get_layer(s_prompt_layer));
      }
      s_timer = app_timer_register(PROMPT_DELTA, set_time_anim, 0);
      break;
  }

  // Finallly
  state++;
}

static void tick_handler(struct tm *t, TimeUnits units_changed) {
  if (s_timer) {
    app_timer_cancel(s_timer);
  }

  // Start anim cycle
  state = 0;
  s_timer = app_timer_register(PROMPT_DELTA, set_time_anim, 0);

  // Blank before time change
  text_layer_set_text(s_time_label, "C:\\>");
  layer_remove_from_parent(text_layer_get_layer(s_time_layer));
  text_layer_set_text(s_date_label, "");
  layer_remove_from_parent(text_layer_get_layer(s_date_layer));
  text_layer_set_text(s_prompt_label, "");

  // Flash
  layer_remove_from_parent(inverter_layer_compat_get_layer(s_prompt_layer));
  prompt_visible = false;

  // Change time display
  set_time(t);
}

/***************************** Window Lifecycle *********************************/

static void window_load(Window *window)  {
  Layer *root_layer = window_get_root_layer(window);

  // Font
  ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_LUCIDA_16);

  // Time
  s_time_label = make_text_layer(GRect(5, 5, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_time_label, "");
  layer_add_child(root_layer, text_layer_get_layer(s_time_label));

  s_time_layer = make_text_layer(GRect(5, 24, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_time_layer, "");
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  // Date
  s_date_label = make_text_layer(GRect(5, 53, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_date_label, "");
  layer_add_child(root_layer, text_layer_get_layer(s_date_label));

  s_date_layer = make_text_layer(GRect(5, 70, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_date_layer, "");
  layer_add_child(root_layer, text_layer_get_layer(s_date_layer));

  // Prompt
  s_prompt_label = make_text_layer(GRect(5, 99, 144, 30), GColorWhite, GColorClear, font_handle, GTextAlignmentLeft);
  text_layer_set_text(s_prompt_label, "");
  layer_add_child(root_layer, text_layer_get_layer(s_prompt_label));

  s_prompt_layer = inverter_layer_compat_create(GRect(45, 115, 12, 2), GColorWhite, GColorBlack);
}

static void window_unload(Window *window)  {
  // Time
  text_layer_destroy(s_time_label);
  text_layer_destroy(s_time_layer);

  // Date
  text_layer_destroy(s_date_label);
  text_layer_destroy(s_date_layer);

  // Prompt
  text_layer_destroy(s_prompt_label);
  inverter_layer_compat_destroy(s_prompt_layer);
}

/******************************** App Lifecycle *********************************/

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
    window_set_background_color(s_window, GColorBlack);
  }

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window_stack_push(s_window, true);
}
