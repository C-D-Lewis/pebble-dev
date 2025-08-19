#include "main_window.h"

// #define TEST

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

/*********************************** Drawing **********************************/

static void canvas_layer_update_proc(Layer *layer, GContext *ctx) {
  isometric_begin(ctx);
  isometric_set_projection_offset(GPoint(31, (144 / 2) + 1));

  SimpleTime mode_time = s_animating ? s_anim_time : s_current_time;

  // Offsets for B&W shadow rendering
  drawing_draw_number(mode_time.hour_tens, GPoint(0, 0));
  drawing_draw_number(mode_time.hour_units, GPoint(1, -40));
  drawing_draw_number(mode_time.minute_tens, GPoint(64, 45));
  drawing_draw_number(mode_time.minute_units, GPoint(64, 5));

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
  int hours = tick_time->tm_hour;
  s_current_time.hour_tens = hours / 10;
  s_current_time.hour_units = hours % 10;
  s_current_time.minute_tens = tick_time->tm_min / 10;
  s_current_time.minute_units = tick_time->tm_min % 10;

  // Colors update for day and night
  if (hours >= 6 && hours < 18) {
    drawing_set_colors(GColorBlack, GColorLightGray);
    window_set_background_color(s_window, GColorWhite);
  } else {
    drawing_set_colors(GColorWhite, GColorDarkGray);
    window_set_background_color(s_window, GColorBlack);
  }

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
  // Show initial time
  time_t t = time(NULL);
  struct tm *tm_now = localtime(&t);
  s_current_time.hour_tens = tm_now->tm_hour / 10;
  s_current_time.hour_units = tm_now->tm_hour % 10;
  s_current_time.minute_tens = tm_now->tm_min / 10;
  s_current_time.minute_units = tm_now->tm_min % 10;

  GColor initial_bg_color = tm_now->tm_hour >= 6 && tm_now->tm_hour < 18 ? GColorWhite : GColorBlack;

  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, initial_bg_color);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Begin smooth animation
  static AnimationImplementation anim_implementation = { .update = anim_update };
  animate(1500, 200, &anim_implementation, true);
}
