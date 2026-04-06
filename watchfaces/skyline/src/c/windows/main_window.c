#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;

static void draw_hour_chunk(GContext *ctx, GRect bounds, int hour, int inset) {
  const int chunk_start_angle = (TRIG_MAX_ANGLE * hour) / 24;
  const int chunk_end_angle = (TRIG_MAX_ANGLE * (hour + 1)) / 24;

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
  GRect bounds = layer_get_bounds(layer);
  const int half_w = bounds.size.w / 2;
  const int half_h = bounds.size.h / 2;

  // Conditions chunks outside outer arc according to weather conditions
  for (int i = 0; i < 24; i++) {
    const int code = data_get_strarr_value(data_get_code_arr(), i);
    const GColor color = data_get_weather_color(code);
    
    graphics_context_set_fill_color(ctx, color);
    draw_hour_chunk(ctx, bounds, i, OUTER_RING_INSET - 1);
  }

  // Chunks between arcs according to temp conditions
  for (int i = 0; i < 24; i++) {
    const int temp = data_get_strarr_value(data_get_temp_arr(), i);
    const GColor color = data_get_temp_color(temp);
    
    // draw chunk function!
    graphics_context_set_fill_color(ctx, color);
    draw_hour_chunk(ctx, grect_inset(bounds, GEdgeInsets(TEMP_RING_INSET)), i, TEMP_RING_W);
  }

  // Outer decoration arc
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET)),
    GOvalScaleModeFitCircle,
    0,
    TRIG_MAX_ANGLE
  );

  // Show 0 and 12 on the dial inside arcs
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    "0",
    scl_get_font(SFI_Dial),
    GRect(half_w - 10, scl_y_pp({.e = 180, .c = 150, .g = 110}), 20, 20),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_draw_text(
    ctx,
    "12",
    scl_get_font(SFI_Dial),
    GRect(half_w - 10, scl_y_pp({.e = 720, .c = 730, .g = 800}), 20, 20),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Line from center to inner arc for day progress
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  const int day_progress = (((t->tm_hour * 60) + t->tm_min) * 100) / MINUTES_PER_DAY;
  const int progress_angle = (TRIG_MAX_ANGLE * day_progress) / 100;
  const int line_length = half_w - INNER_RING_INSET;
  const int line_end_x = half_w + (line_length * sin_lookup(progress_angle)) / TRIG_MAX_RATIO;
  const int line_end_y = half_h - (line_length * cos_lookup(progress_angle)) / TRIG_MAX_RATIO;
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_line(ctx, GPoint(half_w, half_h), GPoint(line_end_x, line_end_y));

  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(INNER_RING_INSET -1)),
    GOvalScaleModeFitCircle,
    0,
    progress_angle
  );

  // Time
  static char time_buff[6];
  strftime(time_buff, sizeof(time_buff), "%H:%M", t);
  const GRect time_bounds = GRect(
    0,
    scl_y_pp({.c = 240, .e = 280, .g = 240}),
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
  const int sep_y = scl_y_pp({.c = 490, .e = 490, .g = 495});
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(
    ctx,
    GRect(bounds.size.w / 4, sep_y, half_w, SEP_H),
    0,
    GCornersAll
  );

  // Battery fills separator rect
  const BatteryChargeState state = battery_state_service_peek();
  graphics_context_set_fill_color(ctx, GColorGreen);
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
    scl_y_pp({.c = 510, .e = 525, .g = 520}),
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
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Get weather every 1h
  if (tick_time->tm_min == 0) comm_request_weather();

  layer_mark_dirty(s_canvas_layer);
}

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

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, MINUTE_UNIT);
}

void main_window_reload() {
  layer_mark_dirty(s_canvas_layer);
}
