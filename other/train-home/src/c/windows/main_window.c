#include "main_window.h"

#define COLOR_BACKGROUND PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorWhite)
#define COLOR_FOREGROUND PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack)

static Window *s_window;
static StatusBarLayer *s_status_bar;
static ContentIndicator *s_indicator;
static TextLayer *s_departs_layer, *s_departs_layer, *s_details_layer, *s_mode_layer;
static Layer *s_up_layer, *s_down_layer;

static int s_index;
static bool s_automatic;

static int time_to_stamp(int hour, int min) {
  return (hour * 60) + min;
}

static void find_next() {
  time_t now = time(NULL);
  struct tm *time_now = localtime(&now);
  int now_stamp = time_to_stamp(time_now->tm_hour, time_now->tm_min);
  for(int i = 0; i < data_get_num_services(); i++) {
    Service *s = data_get_service(i);

    // Is this train the next latest?
    if(time_to_stamp(s->depart_hour, s->depart_minute) > now_stamp) {
      if(i != s_index) {
        vibes_double_pulse();  // Notify a train has departed
      }

      s_index = i;
      return;
    }
  }
}

static int time_difference(int hour_d, int min_d, int hour_n, int min_n) {
  return time_to_stamp(hour_d, min_d) - time_to_stamp(hour_n, min_n);
}

static void update_data() {
  content_indicator_set_content_available(s_indicator, ContentIndicatorDirectionUp, (s_index > 0));
  content_indicator_set_content_available(s_indicator, ContentIndicatorDirectionDown, (s_index < data_get_num_services() - 1));

  Service *service = data_get_service(s_index);
  
  // Departure time
  static char s_departs_buff[8];
  if(service->depart_minute < 10) {
    snprintf(s_departs_buff, sizeof(s_departs_buff), "%d:0%d", service->depart_hour, service->depart_minute);
  } else {
    snprintf(s_departs_buff, sizeof(s_departs_buff), "%d:%d", service->depart_hour, service->depart_minute);
  }
  text_layer_set_text(s_departs_layer, s_departs_buff);

  // Other details
  static char s_time_buff[8];
  if(service->arrive_minute < 10) {
    snprintf(s_time_buff, sizeof(s_time_buff), "%d:0%d", service->arrive_hour, service->arrive_minute);
  } else {
    snprintf(s_time_buff, sizeof(s_time_buff), "%d:%d", service->arrive_hour, service->arrive_minute);
  }

  // Time until departure
  time_t now = time(NULL);
  struct tm* time_now = localtime(&now);
  int t_minus = time_difference(service->depart_hour, service->depart_minute, time_now->tm_hour, time_now->tm_min);
  
  // Duration
  int duration = time_difference(service->arrive_hour, service->arrive_minute, service->depart_hour, service->depart_minute);
  if(duration < -1000) {
    // Next day
    duration = 60 - service->depart_minute + service->arrive_minute;
  }

  // Put it all together
  static char s_details_buff[512];
  snprintf(s_details_buff, sizeof(s_details_buff), "Arrives %s (%dm)\nDeparts in %dm\nPlatform %d", &s_time_buff[0], duration, t_minus, service->platform);
  text_layer_set_text(s_details_layer, s_details_buff);
}

static void toggle_auto() {
  s_automatic = !s_automatic;

  text_layer_set_text(s_mode_layer, s_automatic ? "A" : "M");
  vibes_short_pulse();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) { 
  toggle_auto();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) { 
  if(s_index > 0) {
    s_index--;
  }
  update_data();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) { 
  if(s_index < data_get_num_services() - 1) {
    s_index++;
  }
  update_data();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // ContentIndicator
  s_up_layer = layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(window_layer, s_up_layer);
  s_down_layer = layer_create(grect_inset(bounds, GEdgeInsets(bounds.size.h - STATUS_BAR_LAYER_HEIGHT, 0, 0, 0)));
  layer_add_child(window_layer, s_down_layer);
  s_indicator = content_indicator_create();
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionUp, &(ContentIndicatorConfig) {
    .layer = s_up_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = COLOR_FOREGROUND,
      .background = COLOR_BACKGROUND
    }
  });
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionDown, &(ContentIndicatorConfig) {
    .layer = s_down_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = COLOR_FOREGROUND,
      .background = COLOR_BACKGROUND
    }
  });

  // Labels
  int x = PBL_IF_ROUND_ELSE(40, 5);
  int y = PBL_IF_ROUND_ELSE(40, 28);
  s_departs_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(y, 0, 0, x)));
  text_layer_set_font(s_departs_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_departs_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(s_departs_layer, GColorClear);
  text_layer_set_text(s_departs_layer, "Departs:");
  layer_add_child(window_layer, text_layer_get_layer(s_departs_layer));

  y += 12;
  s_departs_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(y, 0, 0, x)));
  text_layer_set_font(s_departs_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_color(s_departs_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(s_departs_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_departs_layer));

  y += 48;
  s_details_layer = text_layer_create(grect_inset(bounds, GEdgeInsets(y, 0, 0, x)));
  text_layer_set_font(s_details_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_details_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(s_details_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_details_layer));

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_bar, COLOR_BACKGROUND, COLOR_FOREGROUND);
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

  y = PBL_IF_ROUND_ELSE(23, 11);
  x = PBL_IF_ROUND_ELSE(130, 120);
  s_mode_layer = text_layer_create(GRect(x, y, bounds.size.w, 40));
  text_layer_set_font(s_mode_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_color(s_mode_layer, COLOR_FOREGROUND);
  text_layer_set_background_color(s_mode_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_mode_layer));
}

static void window_unload(Window *window) {
  content_indicator_destroy(s_indicator);
  layer_destroy(s_up_layer);
  layer_destroy(s_down_layer);
  text_layer_destroy(s_departs_layer);
  text_layer_destroy(s_departs_layer);
  text_layer_destroy(s_details_layer);
  text_layer_destroy(s_mode_layer);
  status_bar_layer_destroy(s_status_bar);

  window_destroy(s_window);
  s_window = NULL;
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  if(s_automatic) {
    find_next();
  }
  update_data();  // Always update time estimates
}

void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorWhite));
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  find_next();
  update_data();

  s_automatic = false;
  text_layer_set_text(s_mode_layer, "M");
}
