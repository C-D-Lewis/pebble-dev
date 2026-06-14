#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;

typedef struct {
  int hour_tens;
  int hour_units;
  int minute_tens;
  int minute_units;
} SimpleTime;

static SimpleTime s_current_time, s_anim_time;
static bool s_animating = true;

static bool get_is_day() {
#ifdef TEST
  return T_IS_DAY;
#else
  time_t t = time(NULL);
  struct tm *tm_now = localtime(&t);
  return tm_now->tm_hour >= 6 && tm_now->tm_hour < 18;
#endif
}

/*********************************** Drawing **********************************/

static void canvas_layer_update_proc(Layer *layer, GContext *ctx) {
  isometric_begin(ctx);
  isometric_set_projection_offset(PROJECTION_OFFSET);

  SimpleTime mode_time = s_animating ? s_anim_time : s_current_time;

#ifdef TEST
  mode_time.hour_tens = T_HOURS / 10;
  mode_time.hour_units = T_HOURS % 10;
  mode_time.minute_tens = T_MINUTES / 10;
  mode_time.minute_units = T_MINUTES % 10;
#endif

  // Offsets for B&W shadow rendering
  drawing_draw_number(mode_time.hour_tens, HOUR_TENS_ORIGIN);
  drawing_draw_number(mode_time.hour_units, HOUR_UNITS_ORIGIN);
  drawing_draw_number(mode_time.minute_tens, MINUTE_TENS_ORIGIN);
  drawing_draw_number(mode_time.minute_units, MINUTE_UNITS_ORIGIN);

  // Finally
  isometric_finish(ctx);
}

/********************************* Animations *********************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (max * dist_normalized) / ANIMATION_NORMALIZED_MAX;
}

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  if (anim) {
    animation_set_duration(anim, duration);
    animation_set_delay(anim, delay);
    animation_set_curve(anim, AnimationCurveEaseInOut);
    animation_set_implementation(anim, implementation);
    if (handlers) {
      animation_set_handlers(anim, (AnimationHandlers) {
        .started = animation_started,
        .stopped = animation_stopped
      }, NULL);
    }
    animation_schedule(anim);
  }
}

static void anim_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hour_tens = anim_percentage(dist_normalized, s_current_time.hour_tens);
  s_anim_time.hour_units = anim_percentage(dist_normalized, s_current_time.hour_units);
  s_anim_time.minute_tens = anim_percentage(dist_normalized, s_current_time.minute_tens);
  s_anim_time.minute_units = anim_percentage(dist_normalized, s_current_time.minute_units);

  layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  PersistData *persist_data = data_get_persist_data();

  const int hours = tick_time->tm_hour;
  s_current_time.hour_tens = hours / 10;
  s_current_time.hour_units = hours % 10;
  s_current_time.minute_tens = tick_time->tm_min / 10;
  s_current_time.minute_units = tick_time->tm_min % 10;

  // Colors update for day and night
  const bool is_day = get_is_day();
  if (is_day) {
    drawing_set_colors(
      persist_data->day_block_color,
      persist_data->day_shadow_color,
      persist_data->day_void_color 
    );
    window_set_background_color(s_window, persist_data->day_bg_color);
  } else {
    drawing_set_colors(
      persist_data->night_block_color,
      persist_data->night_shadow_color,
      persist_data->night_void_color
    );
    window_set_background_color(s_window, persist_data->night_bg_color);
  }

  layer_mark_dirty(s_canvas_layer);
}

static void bt_handler(bool connected) {
  if (!connected) vibes_double_pulse();

  drawing_set_is_connected(connected);
  layer_mark_dirty(s_canvas_layer);
}

/************************************ Window **********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_layer_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  PersistData *persist_data = data_get_persist_data();

  // Show initial time
  time_t t = time(NULL);
  struct tm *tm_now = localtime(&t);
  s_current_time.hour_tens = tm_now->tm_hour / 10;
  s_current_time.hour_units = tm_now->tm_hour % 10;
  s_current_time.minute_tens = tm_now->tm_min / 10;
  s_current_time.minute_units = tm_now->tm_min % 10;

  const bool is_day = get_is_day();
  drawing_set_colors(
    is_day ? persist_data->day_block_color : persist_data->night_block_color,
    is_day ? persist_data->day_shadow_color : persist_data->night_shadow_color,
    is_day ? persist_data->day_void_color : persist_data->night_void_color
  );

  if (!s_window) {
    s_window = window_create();
    window_set_background_color(
      s_window,
      is_day ? persist_data->day_bg_color : persist_data->night_bg_color
    );
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  bluetooth_connection_service_subscribe((BluetoothConnectionHandler)bt_handler);
  drawing_set_is_connected(bluetooth_connection_service_peek());
#ifdef TEST
  drawing_set_is_connected(T_IS_CONNECTED);
#endif

  // Begin smooth animation
  static AnimationImplementation anim_implementation = { .update = anim_update };
  animate(1500, 200, &anim_implementation, true);
}

void main_window_reload() {
  PersistData *persist_data = data_get_persist_data();

  const bool is_day = get_is_day();
  drawing_set_colors(
    is_day ? persist_data->day_block_color : persist_data->night_block_color,
    is_day ? persist_data->day_shadow_color : persist_data->night_shadow_color,
    is_day ? persist_data->day_void_color : persist_data->night_void_color
  );
  window_set_background_color(
    s_window,
    is_day ? persist_data->day_bg_color : persist_data->night_bg_color
  );
  layer_mark_dirty(s_canvas_layer);
}
