#include "main_window.h"

#define Y_ROOT PBL_IF_ROUND_ELSE(scalable_y(330), scalable_y(300))
#define GRECT_HOUR_LAYER GRect(0, Y_ROOT + 3, DISPLAY_W, DISPLAY_H)
#define ARC_RADIUS scalable_x(380)
#define ARC_WIDTH scalable_x(80)
#define ARC_COLOR_DAY PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite)
#define ARC_COLOR_NIGHT PBL_IF_COLOR_ELSE(GColorDukeBlue, GColorWhite)
#define ARC_BG_COLOR PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)
#define ARC_X ((DISPLAY_W - (ARC_RADIUS * 2)) / 2)
#define ARC_Y ((DISPLAY_H - (ARC_RADIUS * 2)) / 2)

static Window *s_window;
static TextLayer *s_hour_layer;
static Layer *s_ring_layer;

static int s_hours = 0;
static int s_minutes = 0;
static int s_animation_progress = 0;

/**
 * Handler when a tick occurs.
 */
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  static char time_buffer[8];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  text_layer_set_text(s_hour_layer, time_buffer);

  s_hours = tick_time->tm_hour;
  s_minutes = tick_time->tm_min;
  layer_mark_dirty(s_ring_layer);
}

/**
 * Draw procedure for the ring layer.
 */
static void ring_layer_update_proc(Layer *layer, GContext *ctx) {
  // Ring background
#if defined(PBL_COLOR)
  graphics_context_set_stroke_width(ctx, ARC_WIDTH);
  graphics_context_set_stroke_color(ctx, ARC_BG_COLOR);
#else
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, GColorWhite);
#endif
  graphics_draw_arc(
    ctx,
    GRect(ARC_X, ARC_Y, ARC_RADIUS * 2, ARC_RADIUS * 2),
    GOvalScaleModeFitCircle,
    0,
    DEG_TO_TRIGANGLE(360)
  );

  // Ring foreground (divide by max, multiple by range)
  const int mins_angle = s_minutes * 6;
  const int progress_angle = ((360 * s_animation_progress) * mins_angle) / (360 * 100);

  bool is_night = s_hours < 6 || s_hours >= 18;
  GColor color = is_night ? ARC_COLOR_NIGHT : ARC_COLOR_DAY;
  graphics_context_set_stroke_width(ctx, ARC_WIDTH);
	graphics_context_set_stroke_color(ctx, color);
  graphics_draw_arc(
    ctx,
    GRect(ARC_X, ARC_Y, ARC_RADIUS * 2, ARC_RADIUS * 2),
    GOvalScaleModeFitCircle,
    0,
    DEG_TO_TRIGANGLE(progress_angle)
  );
}

/**
 * Update during intro animation.
 */
 static void intro_animation_update(Animation *animation, const AnimationProgress progress) {
  s_animation_progress = ((int)progress * 100) / ANIMATION_NORMALIZED_MAX;

  layer_mark_dirty(s_ring_layer);
}

/**
 * Start the intro animation.
 */
static void start_intro_animation() {
  Animation *animation = animation_create();
  animation_set_delay(animation, 300);
  animation_set_duration(animation, 700);
  animation_set_curve(animation, AnimationCurveEaseInOut);
  static AnimationImplementation implementation = {
    .update = (AnimationUpdateImplementation) intro_animation_update
  };
  animation_set_implementation(animation, &implementation);
  animation_schedule(animation);
}


/**
 * Window load event handler.
 */
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = text_layer_create(GRECT_HOUR_LAYER);
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_font(s_hour_layer, scalable_get_font(SFI_Regular));
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));

  s_ring_layer = layer_create(bounds);
  layer_set_update_proc(s_ring_layer, (LayerUpdateProc)ring_layer_update_proc);
  layer_add_child(window_layer, s_ring_layer);
}

/**
 * Window unload event handler.
 */
static void window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);
  layer_destroy(s_ring_layer);

  window_destroy(s_window);
  s_window = NULL;
}

/**
 * Load the main window and push it to the stack.
 */
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

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, MINUTE_UNIT);

  start_intro_animation();
}
