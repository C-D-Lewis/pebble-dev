#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;

static AppTimer *s_tap_timer;
static int s_anim_progress, s_day_prog_angle, s_anim_prog_angle;
static time_t s_last_update_time;
static bool s_tapped = false;

/******************************************** Drawing *********************************************/

static GPoint make_hand_point(int quantity, int intervals, int len, GPoint center) {
  const int angle = (TRIG_MAX_ANGLE * quantity) / intervals;
  return (GPoint) {
    .x = (int16_t)(sin_lookup(angle) * (int32_t)len / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(angle) * (int32_t)len / TRIG_MAX_RATIO) + center.y,
  };
}

static void draw_hour_chunk(GContext *ctx, GRect bounds, int hour, int inset) {
  const int chunk_start_angle = (TRIG_MAX_ANGLE * hour) / 24;
  const int chunk_end_angle = (TRIG_MAX_ANGLE * (hour + 1)) / 24;

  if (chunk_end_angle > s_anim_prog_angle) return;

  graphics_fill_radial(
    ctx,
    bounds,
    GOvalScaleModeFitCircle,
    inset,
    chunk_start_angle,
    chunk_end_angle + 100
  );
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();
  PersistData *persist_data = data_get_persist_data();

  GRect bounds = layer_get_bounds(layer);
  const int half_w = bounds.size.w / 2;
  const int half_h = bounds.size.h / 2;

  // Progress through the day
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
#ifdef TEST
  t->tm_hour = 21;
  t->tm_min = 38;
#endif
  const int day_progress = (((t->tm_hour * 60) + t->tm_min) * 100) / MINUTES_PER_DAY;
  s_day_prog_angle = (TRIG_MAX_ANGLE * day_progress * s_anim_progress) / (100 * 100);

  // Notches
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, INNER_RING_W);
  int notch_x = half_w - 1;
  int notch_y = scl_y_pp({.o = 140, .c = 70, .e = 140, .g = 70});
  graphics_draw_line(ctx, GPoint(notch_x, notch_y), GPoint(notch_x, notch_y + NOTCH_L));
  notch_y = scl_y_pp({.o = 780, .c = 850, .e = 780, .g = 850});
  graphics_draw_line(ctx, GPoint(notch_x, notch_y), GPoint(notch_x, notch_y + NOTCH_L));

  // Radial dots for 'clear' weather segments
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 24; i += 2) {
    GPoint dot_point = make_hand_point(
      i,
      24,
      scl_x_pp({.o = 465, .e = 460, .g = 470}),
      GPoint(half_w, half_h)
    );
    graphics_draw_circle(ctx, dot_point, DOT_S);
  }

  // Weather condition chunks outside outer arc
  for (int i = 0; i < 24; i++) {
    GColor color;
    if (s_tapped) {
      // Precip chance
      const int chance = data_get_strarr_value(app_state->precip_arr, i);
      color = data_get_precip_color(chance);
    } else {
      // General conditions
      const int code = data_get_strarr_value(app_state->code_arr, i);
      color = data_get_weather_color(code);
    }
    
    graphics_context_set_fill_color(ctx, color);
    draw_hour_chunk(
      ctx,
      bounds,
      i,
      OUTER_RING_INSET - 1
    );
  }

  // Chunks between arcs according to temp conditions
  const GRect temp_grect = grect_inset(bounds, GEdgeInsets(TEMP_RING_INSET));
  for (int i = 0; i < 24; i++) {
    const int temp = data_get_strarr_value(app_state->temp_arr, i) - SIGNED_OFFSET;
    const GColor color = data_get_temp_color(temp);
    
    graphics_context_set_fill_color(ctx, color);
    draw_hour_chunk(ctx, temp_grect, i, TEMP_RING_W);
  }

  // Outer decoration arc
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_stroke_color(ctx, s_tapped  ? GColorCobaltBlue : GColorWhite);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET)),
    GOvalScaleModeFitCircle,
    0,
    TRIG_MAX_ANGLE
  );

  // Line from center to inner arc for day progress
  const int line_length = half_w - INNER_RING_INSET + 1;
  const int line_end_x = half_w + (line_length * sin_lookup(s_day_prog_angle)) / TRIG_MAX_RATIO;
  const int line_end_y = half_h - (line_length * cos_lookup(s_day_prog_angle)) / TRIG_MAX_RATIO;
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, INNER_RING_W);
  graphics_draw_line(ctx, GPoint(half_w, half_h), GPoint(line_end_x, line_end_y));
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(INNER_RING_INSET -1)),
    GOvalScaleModeFitCircle,
    0,
    s_day_prog_angle
  );
  graphics_context_set_antialiased(ctx, true);

  // Time
  static char time_buff[6];
  strftime(time_buff, sizeof(time_buff), "%H:%M", t);
  const GRect time_bounds = GRect(
    0,
    scl_y_pp({.o = 270, .c = 240, .e = 280, .g = 240}),
    bounds.size.w,
    100
  );
  graphics_context_set_text_color(ctx, GColorDarkGray);
  graphics_draw_text(
    ctx,
    time_buff,
    scl_get_font(SFI_Time),
    grect_inset(time_bounds, GEdgeInsets(3, 0, 0, 0)),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    time_buff,
    scl_get_font(SFI_Time),
    time_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Separator rect
  const int sep_y = scl_y_pp({.o = 485, .c = 500, .e = 490, .g = 495});
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(
    ctx,
    GRect(bounds.size.w / 4, sep_y, half_w, SEP_H),
    0,
    GCornersAll
  );

  // Battery fills separator rect
  const BatteryChargeState state = battery_state_service_peek();
  const bool connected = connection_service_peek_pebble_app_connection();
  graphics_context_set_fill_color(ctx, connected ? GColorGreen : GColorLightGray);
  graphics_fill_rect(
    ctx,
    GRect(bounds.size.w / 4, sep_y, (half_w * state.charge_percent) / 100, SEP_H),
    0,
    GCornersAll
  );

  // Date
  static char date_buff[12];
  strftime(date_buff, sizeof(date_buff), "%d %b %Y", t);
  const GRect date_bounds = GRect(
    0,
    scl_y_pp({.o = 510, .c = 540, .e = 525, .g = 530}),
    bounds.size.w,
    50
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    date_buff,
    scl_get_font(SFI_Date),
    date_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Current conditions
  static char s_current_buff[16];
  int current_code = app_state->current_code;
  if (current_code == WEATHER_ERROR) {
    snprintf(
      s_current_buff,
      sizeof(s_current_buff),
      "WTHR ERR"
    );
  } else if (current_code != DATA_EMPTY) {
    // Convert temp to current unit
    int current_temp = app_state->current_temp;
    if (strcmp(persist_data->temp_unit, "F") == 0) {
      current_temp = (current_temp * 9 / 5) + 32;
    }

    snprintf(
      s_current_buff,
      sizeof(s_current_buff),
      "%d - %s",
      current_temp,
      data_get_weather_str(current_code)
    );
  } else {
    snprintf(
      s_current_buff,
      sizeof(s_current_buff),
      "..."
    );
  }
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    s_current_buff,
    scl_get_font(SFI_Weather),
    GRect(0, scl_y_pp({.o = 610, .c = 640, .e = 610, .g = 650}), PS_DISP_W, 28),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

/******************************************* Animations *******************************************/

/**
 * Update during intro animation.
 */
 static void intro_animation_update(Animation *animation, const AnimationProgress progress) {
  s_anim_progress = ((int)progress * 100) / ANIMATION_NORMALIZED_MAX;
  s_anim_prog_angle = (TRIG_MAX_ANGLE * s_anim_progress) / 100;

  layer_mark_dirty(s_canvas_layer);
}

/**
 * Start the intro animation.
 */
static void start_intro_animation() {
  s_anim_prog_angle = 0;
  s_anim_progress = 0;

  Animation *animation = animation_create();
  animation_set_delay(animation, 100);
  animation_set_duration(animation, 1500);
  animation_set_curve(animation, AnimationCurveEaseInOut);
  static AnimationImplementation implementation = {
    .update = (AnimationUpdateImplementation) intro_animation_update
  };
  animation_set_implementation(animation, &implementation);
  animation_schedule(animation);
}

/******************************************** Handlers ********************************************/

static void tap_timer_callback(void *context) {
  s_tapped = false;

  layer_mark_dirty(s_canvas_layer);
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  if (s_tapped == true) return;

  s_tapped = true;
  layer_mark_dirty(s_canvas_layer);
  s_tap_timer = app_timer_register(3000, tap_timer_callback, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Seeing multiple ticks with MINUTE_UNIT...
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Tick! %d", tick_time->tm_sec);

  time_t now = time(NULL);
  if (tick_time->tm_min == 0 && (now - s_last_update_time) >= MIN_WEATHER_INTERVAL_S) {
    comm_request_weather();
  }

  layer_mark_dirty(s_canvas_layer);
}

static void bt_handler(bool connected) {
  if (!connected) vibes_double_pulse();

  layer_mark_dirty(s_canvas_layer);
}

/********************************************* Window *********************************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_background_color(s_window, GColorBlack);
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  accel_tap_service_subscribe(accel_tap_handler);
}

void main_window_reload() {
  // Weather data or config came in
  s_last_update_time = time(NULL);
  start_intro_animation();
}
