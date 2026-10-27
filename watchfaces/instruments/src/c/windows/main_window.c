#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;

static AppTimer *s_tap_timer;
static bool s_tapped, s_connected;
static int s_anim_progress, s_day_prog_angle, s_anim_prog_angle;
static time_t s_last_update_time;

/******************************************** Drawing *********************************************/

static void reload_bitmaps() {
  // Weather changed
}

static void draw_date_and_time_display(GContext *ctx, GRect bounds, struct tm *t, int half_w, GPoint center) {
  const int day = t->tm_mday;

  // Date - Fri Oct
  static char date_buff[32];
  strftime(
    date_buff,
    sizeof(date_buff),
    day < 10 ? "%a       %b" : "%a          %b",
    t
  );
  const GRect date_bounds = GRect(0, scl_y(600), bounds.size.w, 100);
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    date_buff,
    scl_get_font(SFI_Date),
    date_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Day, larger and centered ('24')
  static char day_buff[8];
  strftime(day_buff, sizeof(day_buff), "%d", t);
  const GRect day_bounds = GRect(0, scl_y(590), bounds.size.w, 100);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    day_buff,
    scl_get_font(SFI_DateLarge),
    day_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Day progress hand
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, DAY_HAND_W);
  const int notch_outer_radius = half_w - OUTER_RING_INSET - 1;
  const int notch_inner_radius = CENTER_POST_W - 1;
  GPoint day_notch_start = util_make_hand_point(
    s_day_prog_angle,
    TRIG_MAX_ANGLE,
    notch_outer_radius,
    center
  );
  GPoint day_notch_end = util_make_hand_point(
    s_day_prog_angle,
    TRIG_MAX_ANGLE,
    notch_inner_radius,
    center
  );
  graphics_draw_line(ctx, day_notch_start, day_notch_end);

}

static void draw_weather(GContext *ctx, GRect bounds) {
  AppState *app_state = data_get_app_state();

  // Weather icon
  const uint32_t weather_icon_id = data_get_weather_icon_id();
  // if (weather_icon) {
  //   const GRect icon_bounds = GRect(scl_x(300), scl_y(300), ICON_SIZE, ICON_SIZE);
  //   graphics_draw_bitmap_in_rect(ctx, weather_icon, icon_bounds);
  // }

  // Temperature
  static char temp_buff[16];
  if (app_state->current_temp == DATA_EMPTY) {
    snprintf(temp_buff, sizeof(temp_buff), " ");
  } else {
    snprintf(temp_buff, sizeof(temp_buff), "%d", app_state->current_temp);
  }
  const GRect temp_bounds = GRect(scl_x(520), scl_y(280), bounds.size.w, 100);
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    temp_buff,
    scl_get_font(SFI_Weather),
    temp_bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_antialiased(ctx, false);

  GRect bounds = layer_get_bounds(layer);
  const int half_w = bounds.size.w / 2;
  const int half_h = bounds.size.h / 2;
  const GPoint center = GPoint(half_w, half_h);

  // Progress through the day
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
#ifdef TEST
  t->tm_hour = 21;
  t->tm_min = 38;
#endif
  const int day_progress = (((t->tm_hour * 60) + t->tm_min) * 100) / MINUTES_PER_DAY;
  s_day_prog_angle = (TRIG_MAX_ANGLE * day_progress * s_anim_progress) / (100 * 100);

  // Adjust angle so noon is at the top, not middnight
  s_day_prog_angle = (s_day_prog_angle - (2 * (TRIG_MAX_ANGLE / 4))) % TRIG_MAX_ANGLE;
  
  // Radial dots for tracking hours
  const int dot_len = scl_x(360);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 24; i += 1) {
    if (i % 2 == 0) {
      // Even hour
      GPoint dot_point = util_make_hand_point(i, 24, dot_len, center);
      graphics_draw_circle(ctx, dot_point, DOT_S);
    } else {
      // Odd hour
      GPoint dot_point = util_make_hand_point(i, 24, dot_len, center);
      graphics_draw_circle(ctx, dot_point, DOT_S / 2);
    }
  }
  
  // Radial notches

  // Outer decoration arc
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET)),
    GOvalScaleModeFitCircle,
    0,
    TRIG_MAX_ANGLE
  );

  draw_weather(ctx, bounds);

  // Tapped state?
  if (s_tapped) {
    // Show seconds?
  }
  
  // Center circle
  if (s_connected) {
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_circle(ctx, center, 2 * CENTER_POST_W);
  } else {
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, center, 2 * CENTER_POST_W);
  }
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, CENTER_POST_W);

  draw_date_and_time_display(ctx, bounds, t, half_w, center);
}

/******************************************* Animations *******************************************/

 static void intro_animation_update(Animation *animation, const AnimationProgress progress) {
  s_anim_progress = ((int)progress * 100) / ANIMATION_NORMALIZED_MAX;
  s_anim_prog_angle = (TRIG_MAX_ANGLE * s_anim_progress) / 100;

  layer_mark_dirty(s_canvas_layer);
}

static void start_intro_animation() {
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

  s_tap_timer = app_timer_register(5000, tap_timer_callback, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  time_t now = time(NULL);
  if (tick_time->tm_min == 0 && (now - s_last_update_time) >= MIN_WEATHER_INTERVAL_S) {
    comm_request_weather();
  }

  layer_mark_dirty(s_canvas_layer);
}

static void bt_handler(bool connected) {
  s_connected = connected;
  if (!connected) vibes_double_pulse();

  layer_mark_dirty(s_canvas_layer);
}

/********************************************* Window *********************************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  reload_bitmaps();

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  // Destroy all bitmaps here

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
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

  s_connected = bluetooth_connection_service_peek();
}

// Weather data or config came in
void main_window_reload() {
  s_last_update_time = time(NULL);

  reload_bitmaps();
  layer_mark_dirty(s_canvas_layer);

  // Update display
  s_anim_prog_angle = 0;
  s_anim_progress = 0;
  start_intro_animation();
}
