#include "main_window.h"

#define MARGIN             5
#define THICKNESS          3
#define ANIMATION_DELAY    300
#define ANIMATION_DURATION 1000
#define HAND_LENGTH_SEC    65
#define HAND_LENGTH_MIN    HAND_LENGTH_SEC
#define HAND_LENGTH_HOUR   (HAND_LENGTH_SEC - 20)

typedef struct {
  int days;
  int hours;
  int minutes;
  int seconds;
} SimpleTime;

static Window *s_main_window;
static TextLayer *s_weekday_layer, *s_day_in_month_layer, *s_month_layer, *s_step_layer;
static Layer *s_canvas_layer, *s_bg_layer;

// One each of these to represent the current time and an animated pseudo-time
static SimpleTime s_current_time, s_anim_time;

static char s_weekday_buffer[8], s_month_buffer[8], s_day_buffer[3], s_current_steps_buffer[16];
static bool s_animating, s_is_connected;

static void center_step_layer();

/******************************* Event Services *******************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  s_current_time.days = tick_time->tm_mday;
  s_current_time.hours = tick_time->tm_hour;
  s_current_time.minutes = tick_time->tm_min;
  s_current_time.seconds = tick_time->tm_sec;

  s_current_time.hours -= (s_current_time.hours > 12) ? 12 : 0;

  snprintf(s_day_buffer, sizeof(s_day_buffer), "%d", s_current_time.days);
  strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%a", tick_time);
  strftime(s_month_buffer, sizeof(s_month_buffer), "%b", tick_time);
  
  int steps = get_step_count();
  memset(s_current_steps_buffer, 0, strlen(s_current_steps_buffer));
  while (steps > 0) {
    int digit = steps % 10;
    steps = steps / 10;
    char buffer[4];
    snprintf(buffer, sizeof(buffer), "%d\n", digit);
    strcat(buffer, s_current_steps_buffer);
    strcpy(s_current_steps_buffer, buffer);
  }

  text_layer_set_text(s_weekday_layer, s_weekday_buffer);
  text_layer_set_text(s_day_in_month_layer, s_day_buffer);
  text_layer_set_text(s_month_layer, s_month_buffer);
  text_layer_set_text(s_step_layer, s_current_steps_buffer);
  center_step_layer();

  // Finally
  layer_mark_dirty(s_canvas_layer);
}

static void bt_handler(bool connected) {
  // Notify disconnection
  if(!connected && s_is_connected) {
    vibes_long_pulse();
  }

  s_is_connected = connected;
  layer_mark_dirty(s_canvas_layer);
}

static void batt_handler(BatteryChargeState state) {
  layer_mark_dirty(s_canvas_layer);
}

/************************** AnimationImplementation ***************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;

  main_window_reload_config();
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  if(anim) {
    animation_set_duration(anim, duration);
    animation_set_delay(anim, delay);
    animation_set_curve(anim, AnimationCurveEaseInOut);
    animation_set_implementation(anim, implementation);
    if(handlers) {
      animation_set_handlers(anim, (AnimationHandlers) {
        .started = animation_started,
        .stopped = animation_stopped
      }, NULL);
    }
    animation_schedule(anim);
  }
}

/****************************** Drawing Functions *****************************/

static void bg_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  BatteryChargeState state = battery_state_service_peek();
  int perc = state.charge_percent;
  int batt_hours = (int)(12.0F * ((float)perc / 100.0F)) + 1;

  for(int h = 0; h < 12; h++) {
    for(int y = 0; y < THICKNESS; y++) {
      for(int x = 0; x < THICKNESS; x++) {
        GPoint point = (GPoint) {
          .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * h / 12) * (int32_t)(3 * HAND_LENGTH_SEC) / TRIG_MAX_RATIO) + center.x,
          .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * h / 12) * (int32_t)(3 * HAND_LENGTH_SEC) / TRIG_MAX_RATIO) + center.y,
        };

        if(data_get(DataKeyBattery)) {
          if(h < batt_hours) {
#ifdef PBL_COLOR
            if(state.is_plugged) {
              // Charging
              graphics_context_set_stroke_color(ctx, GColorGreen);
            } else {
              // Discharging at this level
              graphics_context_set_stroke_color(ctx, GColorWhite);
            }
#else
            graphics_context_set_stroke_color(ctx, GColorWhite);
#endif
          } else {
            // Empty segment
            graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack));
          }
        } else {
          // No battery indicator, show all
          graphics_context_set_stroke_color(ctx, GColorWhite);
        }
        graphics_draw_line(ctx, GPoint(center.x + x, center.y + y), GPoint(point.x + x, point.y + y));
      }
    }
  }

  // Make markers
  graphics_context_set_fill_color(ctx, GColorBlack);
#ifdef PBL_ROUND
  graphics_fill_circle(ctx, center, (bounds.size.w / 2) - (2 * MARGIN));
#else
  graphics_fill_rect(ctx, GRect(MARGIN, MARGIN, bounds.size.w - (2 * MARGIN), bounds.size.h - (2 * MARGIN)), 0, GCornerNone);
#endif
}

static GPoint make_hand_point(int quantity, int intervals, int len, GPoint center) {
  return (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * quantity / intervals) * (int32_t)len / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * quantity / intervals) * (int32_t)len / TRIG_MAX_RATIO) + center.y,
  };
}

static int hours_to_minutes(int hours_out_of_12) {
  return (hours_out_of_12 * 60) / 12;
}

static void draw_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);  

  SimpleTime mode_time = (s_animating) ? s_anim_time : s_current_time;

  int len_sec = HAND_LENGTH_SEC;
  int len_min = HAND_LENGTH_MIN;
  int len_hour = HAND_LENGTH_HOUR;

  // Plot hand ends
  GPoint second_hand_long = make_hand_point(mode_time.seconds, 60, len_sec, center);
  GPoint minute_hand_long = make_hand_point(mode_time.minutes, 60, len_min, center);

  // Plot shorter overlaid hands
  len_sec -= (MARGIN + 2);
  len_min -= (MARGIN + 2);
  GPoint second_hand_short = make_hand_point(mode_time.seconds, 60, len_sec, center);
  GPoint minute_hand_short = make_hand_point(mode_time.minutes, 60, len_min, center);

  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if(s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Hour is more accurate
  GPoint hour_hand_long = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)len_hour / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)len_hour / TRIG_MAX_RATIO) + center.y,
  };

  // Shorter hour overlay
  len_hour -= (MARGIN + 2);
  GPoint hour_hand_short = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)len_hour / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)len_hour / TRIG_MAX_RATIO) + center.y,
  };

  // Draw hands
  graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite));
  for(int y = 0; y < THICKNESS; y++) {
    for(int x = 0; x < THICKNESS; x++) {
      graphics_draw_line(ctx, GPoint(center.x + x, center.y + y), GPoint(minute_hand_short.x + x, minute_hand_short.y + y));
      graphics_draw_line(ctx, GPoint(center.x + x, center.y + y), GPoint(hour_hand_short.x + x, hour_hand_short.y + y));
    }
  }
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for(int y = 0; y < THICKNESS; y++) {
    for(int x = 0; x < THICKNESS; x++) {
      graphics_draw_line(ctx, GPoint(minute_hand_short.x + x, minute_hand_short.y + y), GPoint(minute_hand_long.x + x, minute_hand_long.y + y));
      graphics_draw_line(ctx, GPoint(hour_hand_short.x + x, hour_hand_short.y + y), GPoint(hour_hand_long.x + x, hour_hand_long.y + y));
    }
  }

  // Draw second hand
  if(data_get(DataKeySecondHand)) {
    // Use loops
    for(int y = 0; y < THICKNESS - 1; y++) {
      for(int x = 0; x < THICKNESS - 1; x++) {
        graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorWhite));
        graphics_draw_line(ctx, GPoint(center.x + x, center.y + y), GPoint(second_hand_short.x + x, second_hand_short.y + y));

        // Draw second hand tip
        graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite));
        graphics_draw_line(ctx, GPoint(second_hand_short.x + x, second_hand_short.y + y), GPoint(second_hand_long.x + x, second_hand_long.y + y));
      }
    }
  }

  // Center
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(center.x + 1, center.y + 1), 4);

  // Draw black if disconnected
  if(data_get(DataKeyBT) && !s_is_connected) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(center.x + 1, center.y + 1), 3);
  }
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_bg_layer);

  int x_offset = (bounds.size.w * 62) / 100;
  int y_offset = (bounds.size.h / 2) - 33;

  s_weekday_layer = text_layer_create(GRect(x_offset, y_offset + 5, 44, 40));
  text_layer_set_text_alignment(s_weekday_layer, GTextAlignmentCenter);
  text_layer_set_font(s_weekday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_weekday_layer, GColorWhite);
  text_layer_set_background_color(s_weekday_layer, GColorClear);

  s_day_in_month_layer = text_layer_create(GRect(x_offset, y_offset + 18, 44, 40));
  text_layer_set_text_alignment(s_day_in_month_layer, GTextAlignmentCenter);
  text_layer_set_font(s_day_in_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_day_in_month_layer, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite));
  text_layer_set_background_color(s_day_in_month_layer, GColorClear);

  s_month_layer = text_layer_create(GRect(x_offset, y_offset + 45, 44, 40));
  text_layer_set_text_alignment(s_month_layer, GTextAlignmentCenter);
  text_layer_set_font(s_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(s_month_layer, GColorWhite);
  text_layer_set_background_color(s_month_layer, GColorClear);

  s_step_layer = text_layer_create(GRect(0, 0, 44, 90));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_color(s_step_layer, GColorWhite);
  text_layer_set_background_color(s_step_layer, GColorClear);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, draw_proc);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  layer_destroy(s_bg_layer);

  text_layer_destroy(s_weekday_layer);
  text_layer_destroy(s_day_in_month_layer);
  text_layer_destroy(s_month_layer);
  text_layer_destroy(s_step_layer);

  window_destroy(s_main_window);
}

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (max * dist_normalized) / ANIMATION_NORMALIZED_MAX;
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_current_time.hours -= (s_current_time.hours > 12) ? 12 : 0;

  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_current_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_current_time.minutes);
  s_anim_time.seconds = anim_percentage(dist_normalized, s_current_time.seconds);

  layer_mark_dirty(s_canvas_layer);
}

static void center_step_layer() {
  if (!data_get(DataKeySteps) || !is_health_available()) {
    return;
  }

  Layer *step_layer_root = text_layer_get_layer(s_step_layer);
  Layer *window_layer = window_get_root_layer(layer_get_window(step_layer_root));
  GRect bounds = layer_get_bounds(window_layer);

  int x_offset = (bounds.size.w * 62) / 100;

  int length = 0;
  if (step_data_is_available()) {
    // Determine the length of steps so we can vertically center the step count
    int steps = get_step_count();
    length = 0;
    while (steps > 0) {
      steps /= 10;
      length++;
    }
  }
  
  layer_set_frame(step_layer_root, GRect(bounds.size.w - (x_offset+54), 72 - (length-1)*9 , 44, 90));
}

/************************************ API *************************************/

void main_window_push() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  // Begin smooth animation
  static AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);

  main_window_reload_config();
}

void main_window_reload_config() {
  time_t t = time(NULL);
  struct tm *tm_now = localtime(&t);
  s_current_time.hours = tm_now->tm_hour;
  s_current_time.minutes = tm_now->tm_min;
  s_current_time.seconds = tm_now->tm_sec;  

  tick_timer_service_unsubscribe();
  if(data_get(DataKeySecondHand)) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }  

  connection_service_unsubscribe();
  if(data_get(DataKeyBT)) {
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bt_handler
    });
    bt_handler(connection_service_peek_pebble_app_connection());
  }

  battery_state_service_unsubscribe();
  if(data_get(DataKeyBattery)) {
    battery_state_service_subscribe(batt_handler);
  }

  Layer *window_layer = window_get_root_layer(s_main_window);
  layer_remove_from_parent(text_layer_get_layer(s_day_in_month_layer));
  layer_remove_from_parent(text_layer_get_layer(s_weekday_layer));
  layer_remove_from_parent(text_layer_get_layer(s_month_layer));
  layer_remove_from_parent(text_layer_get_layer(s_step_layer));
  if(data_get(DataKeyDay)) {
    layer_add_child(window_layer, text_layer_get_layer(s_day_in_month_layer));
  }
  if(data_get(DataKeyDate)) {
    layer_add_child(window_layer, text_layer_get_layer(s_weekday_layer));
    layer_add_child(window_layer, text_layer_get_layer(s_month_layer));
  }
  if(data_get(DataKeySteps) && is_health_available()) {
    layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
  }

  layer_add_child(window_layer, s_canvas_layer);
}

