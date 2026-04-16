#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;

static AppTimer *s_tap_timer;
static GBitmap *s_temp_bitmap, *s_wind_bitmap, *s_humidity_bitmap;
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

static int convert_temp(PersistData *persist_data, int val_c) {
  return strcmp(persist_data->temp_unit, "F") == 0
    ? (val_c * 9 / 5) + 32
    : val_c;
}

static int convert_wind_speed(PersistData *persist_data, int val_kph) {
  return strcmp(persist_data->wind_unit, WIND_UNIT_MPH) == 0
    ? (val_kph * 1000) / 1609
    : val_kph;
}

static void draw_hour_chunk(GContext *ctx, GRect bounds, int hour, int inset, bool pattern) {
  const int chunk_start_angle = (TRIG_MAX_ANGLE * hour) / 24;
  const int chunk_end_angle = (TRIG_MAX_ANGLE * (hour + 1)) / 24;

  if (chunk_end_angle > s_anim_prog_angle) return;

  if (pattern) {
    const int chunk_size = chunk_end_angle - chunk_start_angle;
    const int stroke_size = chunk_size / NUM_PATTERN_STROKES;
    // Slim slices between start and end
    for (int i = 0; i < NUM_PATTERN_STROKES; i++) {
      if (i % 2 == 0) continue;

      const int slice_start_angle = chunk_start_angle + (i * stroke_size);
      const int slice_end_angle = slice_start_angle + stroke_size;
      graphics_fill_radial(
        ctx,
        bounds,
        GOvalScaleModeFitCircle,
        inset,
        slice_start_angle,
        slice_end_angle
      );
    }
  } else {
    // All in one chunk
    graphics_fill_radial(
      ctx,
      bounds,
      GOvalScaleModeFitCircle,
      inset,
      chunk_start_angle,
      chunk_end_angle + 100
    );
  }
}

static void draw_outer_time_marker(GContext *ctx, GRect bounds, char* str, GColor color) {
  if (strlen(str) == 0) return;

  const char h_arr[3] = {str[0], str[1], '\0'};
  const char m_arr[3] = {str[3], str[4], '\0'};
  const int hour = atoi(h_arr);
  const int mins = atoi(m_arr);
  const int angle = ((hour * 60) + mins) * TRIG_MAX_ANGLE / MINUTES_PER_DAY;
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_radial(
    ctx,
    bounds,
    GOvalScaleModeFitCircle,
    OUTER_RING_INSET,
    angle - 300,
    angle + 300
  );
}

static void draw_time_display(GContext *ctx, GRect bounds, struct tm *t) {
  const int half_w = bounds.size.w / 2;

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
  const int sep_y = scl_y_pp({.o = 485, .c = 490, .e = 485, .g = 495});
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 4, sep_y, half_w, SEP_H), 0, GCornersAll);

  // Battery fills separator rect
  const BatteryChargeState state = battery_state_service_peek();
  const bool connected = connection_service_peek_pebble_app_connection();
  graphics_context_set_fill_color(
    ctx,
    connected ? PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite) : GColorLightGray
  );
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
    scl_y_pp({.o = 510, .c = 510, .e = 505, .g = 530}),
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

static void draw_weather_display(GContext *ctx, GRect bounds) {
  PersistData *persist_data = data_get_persist_data();
  AppState *app_state = data_get_app_state();

  if (app_state->current_code == DATA_EMPTY) return;

  // Current conditions
  static char s_current_buff[16];
  int current_code = app_state->current_code;
  if (current_code == WEATHER_ERROR) {
    snprintf(s_current_buff, sizeof(s_current_buff), "WTHR ERR");
  } else if (current_code != DATA_EMPTY) {
    const int current_temp = convert_temp(persist_data, app_state->current_temp);
    snprintf(
      s_current_buff,
      sizeof(s_current_buff),
      "%d - %s",
      current_temp,
      data_get_weather_str(current_code)
    );
  } else {
    snprintf(s_current_buff, sizeof(s_current_buff), "...");
  }
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_current_buff,
    scl_get_font(SFI_Weather),
    GRect(0, scl_y_pp({.o = 235, .c = 220, .e = 260, .g = 210}), PS_DISP_W, 28),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  const int text_x = scl_x_pp({.o = 410, .c = 380, .e = 420, .g = 350});
  const int icon_x = scl_x_pp({.o = 220, .c = 220, .e = 260, .g = 220});

  // Low / high in readable colors
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(
    ctx,
    s_temp_bitmap,
    GRect(icon_x, scl_y_pp({.o = 340, .c = 360, .e = 370, .g = 350}), ICON_SIZE, ICON_SIZE)
  );
  static char s_low_high_buff[16];
  snprintf(
    s_low_high_buff,
    sizeof(s_low_high_buff),
    "%d / %d",
    convert_temp(persist_data, data_get_min_temp()),
    convert_temp(persist_data, data_get_max_temp())
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_low_high_buff,
    scl_get_font(SFI_Weather),
    GRect(text_x, scl_y_pp({.o = 355, .c = 360, .e = 380, .g = 350}), PS_DISP_W, 28),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );

  // Wind speed
  graphics_draw_bitmap_in_rect(
    ctx,
    s_wind_bitmap,
    GRect(icon_x, scl_y_pp({.o = 490, .c = 510, .e = 510, .g = 490}), ICON_SIZE, ICON_SIZE)
  );
  static char s_wind_buff[16];
  const char *wind_unit = strcmp(persist_data->wind_unit, WIND_UNIT_MPH) == 0 ? "mph" : "km/h";
  snprintf(
    s_wind_buff,
    sizeof(s_wind_buff),
    "%d %s",
    convert_wind_speed(persist_data, app_state->current_wind_kmh),
    wind_unit
  );
  graphics_draw_text(
    ctx,
    s_wind_buff,
    scl_get_font(SFI_Weather),
    GRect(text_x, scl_y_pp({.o = 505, .c = 510, .e = 510, .g = 480}), PS_DISP_W, 28),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );

  // Humidity
  graphics_draw_bitmap_in_rect(
    ctx,
    s_humidity_bitmap,
    GRect(icon_x, scl_y_pp({.o = 630, .c = 650, .e = 630, .g = 630}), ICON_SIZE, ICON_SIZE)
  );
  static char s_humidity_buff[16];
  // TODO: Setting for humidity units? Although they are equivalent
  snprintf(s_humidity_buff, sizeof(s_humidity_buff), "%d %%", app_state->current_humidity_perc);
  graphics_draw_text(
    ctx,
    s_humidity_buff,
    scl_get_font(SFI_Weather),
    GRect(text_x, scl_y_pp({.o = 645, .c = 650, .e = 630, .g = 620}), PS_DISP_W, 28),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();

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

  // Radial dots for 'clear' weather segments
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 24; i += 2) {
    GPoint dot_point = make_hand_point(i, 24, scl_x_pp({.o = 465, .e = 460, .g = 470}), center);
    graphics_draw_circle(ctx, dot_point, DOT_S);
  }

  // Background color
  graphics_context_set_fill_color(ctx, data_get_bg_color());
  graphics_fill_circle(ctx, center, half_w - INNER_RING_INSET);

  // Notches
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, INNER_RING_W);
  int notch_x = half_w - 1;
  int notch_y = scl_y_pp({.o = 140, .c = 70, .e = 140, .g = 70});
  graphics_draw_line(ctx, GPoint(notch_x, notch_y), GPoint(notch_x, notch_y + NOTCH_L));
  notch_y = scl_y_pp({.o = 780, .c = 850, .e = 780, .g = 850});
  graphics_draw_line(ctx, GPoint(notch_x, notch_y), GPoint(notch_x, notch_y + NOTCH_L));

  // Precip chunks background
  if (s_tapped) {
    graphics_context_set_fill_color(ctx, GColorOxfordBlue);
    graphics_fill_radial(
      ctx,
      bounds,
      GOvalScaleModeFitCircle,
      OUTER_RING_INSET - 1,
      0,
      s_anim_prog_angle
    );
  }

  // Weather condition chunks outside outer arc
  for (int i = 0; i < 24; i++) {
    GColor color;
    int thickness = OUTER_RING_INSET - 1;
    GRect chunk_bounds = bounds;

    if (s_tapped) {
      // Precip chance (fixed color, varying height)
      const int chance = data_get_strarr_value(app_state->precip_arr, i);
      color = GColorVividCerulean;

      const int min_chance = 10;
      const int min_h = 3;
      // Scale the remaining pixels (99 is max sent from JS)
      thickness = (chance > min_chance)
        ? min_h + (((OUTER_RING_INSET - min_h) * chance) / 99)
        : 0;
      // Outer bounds have to chance to draw 'outwards' from center instead of shrinking arc
      chunk_bounds = grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET - thickness - 2));
    } else {
      // General conditions
      const int code = data_get_strarr_value(app_state->code_arr, i);
      color = data_get_weather_color(code);
    }
    
    // Use pattern for cloudy segments
    const bool pttrn = gcolor_equal(color, GColorLightGray) || gcolor_equal(color, GColorDarkGray);
    graphics_context_set_fill_color(ctx, color);
    draw_hour_chunk(ctx, chunk_bounds, i, thickness, pttrn);
  }

  if (!s_tapped) {
    // Chunks between arcs according to temp conditions
    const GRect temp_grect = grect_inset(bounds, GEdgeInsets(TEMP_RING_INSET));
    for (int i = 0; i < 24; i++) {
      const int temp = data_get_strarr_value(app_state->temp_arr, i) - SIGNED_OFFSET;
      const GColor color = data_get_temp_color(temp);
      
      graphics_context_set_fill_color(ctx, color);
      draw_hour_chunk(ctx, temp_grect, i, TEMP_RING_W, false);
    }
  }

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

  if (!s_tapped) {
    // Markers for sunrise/sunset
    draw_outer_time_marker(
      ctx,
      bounds,
      app_state->sunrise,
      PBL_IF_COLOR_ELSE(GColorFolly, GColorWhite)
    );
    draw_outer_time_marker(
      ctx,
      bounds,
      app_state->sunset,
      PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)
    );

    // Line from center to inner arc for day progress
    const int line_length = half_w - INNER_RING_INSET + 1;
    const int line_end_x = half_w + (line_length * sin_lookup(s_day_prog_angle)) / TRIG_MAX_RATIO;
    const int line_end_y = half_h - (line_length * cos_lookup(s_day_prog_angle)) / TRIG_MAX_RATIO;
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, INNER_RING_W);
    graphics_draw_arc(
      ctx,
      grect_inset(bounds, GEdgeInsets(INNER_RING_INSET -1)),
      GOvalScaleModeFitCircle,
      0,
      s_day_prog_angle
    );
    graphics_draw_line(ctx, center, GPoint(line_end_x, line_end_y));
  }

  // TEST
  // s_tapped = true;
  if (!s_tapped) {
    draw_time_display(ctx, bounds, t);
  } else {
    draw_weather_display(ctx, bounds);
  }
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

  PersistData *persist_data = data_get_persist_data();

  s_tapped = true;
  layer_mark_dirty(s_canvas_layer);

  // Timeout as per user's preference
  s_tap_timer = app_timer_register(persist_data->tap_timeout * 1000, tap_timer_callback, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
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

  s_temp_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_TEMP);
  s_wind_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_WIND);
  s_humidity_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_HUMIDITY);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  gbitmap_destroy(s_temp_bitmap);
  gbitmap_destroy(s_wind_bitmap);
  gbitmap_destroy(s_humidity_bitmap);

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

// Weather data or config came in
void main_window_reload() {
  PersistData *persist_data = data_get_persist_data();

  s_last_update_time = time(NULL);

  if (strcmp(persist_data->animations, "true") == 0) {
    s_anim_prog_angle = 0;
    s_anim_progress = 0;
    start_intro_animation();
  } else {
    // Instant update
    s_anim_prog_angle = TRIG_MAX_ANGLE;
    s_anim_progress = 100;
    layer_mark_dirty(s_canvas_layer);
  }
}
