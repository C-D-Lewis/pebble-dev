#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;
static GBitmap *s_weather_bitmap;

static AppTimer *s_tap_timer;
static bool s_tapped, s_connected;
static int s_day_prog_ang, s_steps;

/******************************************** Drawing *********************************************/

static void draw_date_and_time_display(GContext *ctx, GRect bounds, struct tm *t, GPoint center) {
  // Day of week
  static char dow_buff[8];
  strftime(dow_buff, sizeof(dow_buff), "%a", t);
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    dow_buff,
    scl_get_font(SFI_Date),
    GRect(scl_x(620), scl_y(360), 30, bounds.size.h),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Day
  static char day_buff[6];
  strftime(day_buff, sizeof(day_buff), "%d", t);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    day_buff,
    scl_get_font(SFI_DateLarge),
    GRect(scl_x(620), scl_y(430), 30, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Month
  static char month_buff[8];
  strftime(month_buff, sizeof(month_buff), "%b", t);
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    month_buff,
    scl_get_font(SFI_Date),
    GRect(scl_x(620), scl_y(545), 30, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Day progress hand
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, DAY_HAND_W);
  GPoint day_hand_start = util_make_hand_point(
    s_day_prog_ang,
    TRIG_MAX_ANGLE,
    center.x - OUTER_RING_INSET - 1,
    center
  );
  GPoint day_hand_end = util_make_hand_point(
    s_day_prog_ang,
    TRIG_MAX_ANGLE,
    CENTER_POST_W - 1,
    center
  );
  graphics_draw_line(ctx, day_hand_start, day_hand_end);
  graphics_context_set_antialiased(ctx, false);
}

static void draw_weather(GContext *ctx, GRect bounds) {
  AppState *app_state = data_get_app_state();

  // Weather icon
  graphics_draw_bitmap_in_rect(
    ctx,
    s_weather_bitmap,
    GRect(scl_x(230), scl_y(450), ICON_SIZE, ICON_SIZE)
  );

  // Temperature
  static char temp_buff[8];
  if (app_state->current_temp == DATA_EMPTY) {
    snprintf(temp_buff, sizeof(temp_buff), " ");
  } else {
    snprintf(temp_buff, sizeof(temp_buff), "%d", util_convert_temp(app_state->current_temp));
  }
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    temp_buff,
    scl_get_font(SFI_Weather),
    GRect(scl_x(345), scl_y(450), 30, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );
}

static void draw_decorations(GContext *ctx, GRect bounds, GPoint center) {
  // Outer day arc
  graphics_context_set_antialiased(ctx, true);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET)),
    GOvalScaleModeFitCircle,
    0,
    TRIG_MAX_ANGLE
  );
  graphics_context_set_antialiased(ctx, false);

  // Straight line representing midnight
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 2);
  const int eod_ang = (180 * TRIG_MAX_ANGLE) / 360;
  GPoint eod_hand_start = util_make_hand_point(
    eod_ang,
    TRIG_MAX_ANGLE,
    center.x - OUTER_RING_INSET - HOUR_DOT_RING_W,
    center
  );
  GPoint eod_hand_end = util_make_hand_point(
    eod_ang,
    TRIG_MAX_ANGLE,
    CENTER_POST_W - 1,
    center
  );
  graphics_draw_line(ctx, eod_hand_start, eod_hand_end);

  // Radial dots for tracking hours
  const int dot_r = center.x - OUTER_RING_INSET - (HOUR_DOT_RING_W / 2) - 1;
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 24; i += 1) {
    if (i % 3 == 0) {
      // Every 3 hours
      GPoint dot_point = util_make_hand_point(i, 24, dot_r, center);
      graphics_draw_circle(ctx, dot_point, HOUR_DOT_S);
    } else {
      // Other hours
      GPoint dot_point = util_make_hand_point(i, 24, dot_r, center);
      graphics_draw_circle(ctx, dot_point, HOUR_DOT_S / 2);
    }
  }
}

static void draw_center_post(GContext *ctx, GPoint center) {
  // Circle shows BT connection status
  if (s_connected) {
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_circle(ctx, center, 2 * CENTER_POST_W);
  } else {
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_circle(ctx, center, 2 * CENTER_POST_W);
  }

  // Center post
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, CENTER_POST_W);
}

static void draw_sunrise_and_sunset_ring(GContext *ctx, GRect bounds, GPoint center) {
  AppState *app_state = data_get_app_state();
  if (app_state->current_code == DATA_EMPTY) return;

  graphics_context_set_antialiased(ctx, true);

  int s_rise_ang = util_get_time_angle(app_state->sunrise);
  int s_set_ang = util_get_time_angle(app_state->sunset);
  // Similar adjustment to angle so noon is at the top, not midnight
  s_rise_ang = (s_rise_ang - (2 * (TRIG_MAX_ANGLE / 4))) % TRIG_MAX_ANGLE;
  s_set_ang = (s_set_ang - (2 * (TRIG_MAX_ANGLE / 4))) % TRIG_MAX_ANGLE;

  const GRect arc_r = grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET));

  // Day and night arc sections
  graphics_context_set_fill_color(ctx, data_get_theme_day_color());
  graphics_fill_radial(ctx, arc_r, GOvalScaleModeFitCircle, HOUR_DOT_RING_W, s_rise_ang, s_set_ang);
  graphics_context_set_fill_color(ctx, data_get_theme_night_color());
  if (s_rise_ang < s_set_ang) {
    s_rise_ang += TRIG_MAX_ANGLE;
  }
  graphics_fill_radial(ctx, arc_r, GOvalScaleModeFitCircle, HOUR_DOT_RING_W, s_set_ang, s_rise_ang);

  // Dawn notch on each border between sunrise and sunset
  const int notch_ang = TRIG_MAX_ANGLE / 256;
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  graphics_fill_radial(
    ctx,
    arc_r,
    GOvalScaleModeFitCircle,
    HOUR_DOT_RING_W,
    s_rise_ang - notch_ang,
    s_rise_ang + notch_ang
  );
  graphics_fill_radial(
    ctx,
    arc_r,
    GOvalScaleModeFitCircle,
    HOUR_DOT_RING_W,
    s_set_ang - notch_ang,
    s_set_ang + notch_ang
  );

  graphics_context_set_antialiased(ctx, false);
}

static void draw_gauge_dot(GContext *ctx, GPoint center, int angle) {
  // Due to how dots are plotted, needs different offset in different positions
  int offset = 2;
  if (angle > TRIG_MAX_ANGLE / 4 || angle > (3 * TRIG_MAX_ANGLE) / 4) {
    offset = 3;
  }

  GPoint dot_point = util_make_hand_point(
    angle,
    TRIG_MAX_ANGLE,
    center.x - (COMPLICATION_W / 2) - offset,
    center
  );
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, dot_point, GAUGE_DOT_S);
}

static void draw_arc_complication(GContext *ctx, GRect bounds, GPoint center, int slot, char *type) {
  AppState *app_state = data_get_app_state();
  PersistData *persist_data = data_get_persist_data();

  graphics_context_set_antialiased(ctx, true);

  // Slots are 0-3, clockwise from top, 45 degrees in length, diagonally opposite
  int start_ang, end_ang;
  switch (slot) {
    case 0:
      start_ang = TRIG_MAX_ANGLE / 16;
      end_ang = (3 * TRIG_MAX_ANGLE) / 16;
      break;
    case 1:
      start_ang = (5 * TRIG_MAX_ANGLE) / 16;
      end_ang = (7 * TRIG_MAX_ANGLE) / 16;
      break;
    case 2:
      start_ang = (9 * TRIG_MAX_ANGLE) / 16;
      end_ang = (11 * TRIG_MAX_ANGLE) / 16;
      break;
    case 3:
      start_ang = (13 * TRIG_MAX_ANGLE) / 16;
      end_ang = (15 * TRIG_MAX_ANGLE) / 16;
      break;
    default:
      return;
  }

  const GRect arc_r = grect_inset(bounds, GEdgeInsets((COMPLICATION_W / 2) + 3));
  const int inner_w = COMPLICATION_W - (OUTER_RING_W + 3);
  const int sep_ang = 75;
  const int item_l = (45 * TRIG_MAX_ANGLE) / (360 * 6);

  if (strcmp(type, COMP_NONE) == 0) {
    // Just a slim line if no complication in this slot
    graphics_context_set_stroke_width(ctx, OUTER_RING_W);
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
  } else {
    // Outline and black fill for empty gauge
    graphics_context_set_stroke_width(ctx, COMPLICATION_W);
    graphics_context_set_stroke_color(ctx, COLOR_COMPLICATION);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
  }

  // Fill based on battery level
  if (strcmp(type, COMP_BATTERY) == 0) {
    BatteryChargeState charge_state = battery_state_service_peek();
    const int batt_ang = ((end_ang - start_ang) * charge_state.charge_percent) / 100;
    graphics_context_set_stroke_color(ctx, COLOR_BATTERY);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + batt_ang);

    draw_gauge_dot(ctx, center, start_ang + batt_ang);
  }
  
  // Fill based on progress to step goal
  if (strcmp(type, COMP_STEP_COUNT) == 0) {
    const int step_goal = atoi(persist_data->step_goal);
    const int max_steps = s_steps > step_goal ? step_goal : s_steps;
    const int step_ang = ((end_ang - start_ang) * max_steps) / step_goal;
    graphics_context_set_stroke_color(ctx, COLOR_STEP_COUNT);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + step_ang);

    draw_gauge_dot(ctx, center, start_ang + step_ang);
  }

  // Next 6 rain chance or weather codes
  const bool is_next_rain = strcmp(type, COMP_NEXT_RAIN) == 0;
  const bool is_next_weather = strcmp(type, COMP_NEXT_WEATHER) == 0;
  if (is_next_rain || is_next_weather) {
    // Fill ends
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_context_set_stroke_color(ctx, COLOR_COMPLICATION);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
    graphics_context_set_antialiased(ctx, true);


    // Each of six hourly items
    for (int i = 0; i < NUM_FORECAST_ITEMS; i++) {
      GColor color = COLOR_ERROR;
      if (is_next_rain) {
        const int chance = data_get_strarr_value(app_state->precip_arr, i);
        color = data_get_precip_color(chance);
      }
      if (is_next_weather) {
        const int code = data_get_strarr_value(app_state->code_arr, i);
        color = data_get_weather_color(code);
      }

      const int item_ang = start_ang + (i * item_l);
      const GRect item_r = grect_inset(bounds, GEdgeInsets(2 * OUTER_RING_W));
      graphics_context_set_fill_color(ctx, color);
      graphics_fill_radial(
        ctx,
        item_r,
        GOvalScaleModeFitCircle,
        inner_w,
        item_ang,
        item_ang + item_l
      );

      // Separator
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_fill_radial(
        ctx,
        item_r,
        GOvalScaleModeFitCircle,
        inner_w,
        item_ang + item_l - sep_ang,
        item_ang + item_l + sep_ang
      );
    }

    // Be in first item all day until last 6 hours
    time_t temp = time(NULL);
    struct tm *t = localtime(&temp);
    const int hours = t->tm_hour;
    int dot_ang = start_ang + (item_l / 2);
    if (hours > 24 - NUM_FORECAST_ITEMS) {
      // Track dot to the end of day using current hour
      dot_ang += (NUM_FORECAST_ITEMS - (24 - hours)) * item_l;
    }
    draw_gauge_dot(ctx, center, dot_ang);
  }

  if (strcmp(type, COMP_TEMP_RANGE) == 0) {
    const int cap_ang = 300;
    const GRect blk_arc_r = grect_inset(bounds, GEdgeInsets(2 * OUTER_RING_W));

    // Fill ends with cool/hot colors and middle with black
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_context_set_stroke_color(ctx, COLOR_HIGH_TEMP);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
    graphics_context_set_stroke_color(ctx, COLOR_LOW_TEMP);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + cap_ang);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + cap_ang,
      end_ang - cap_ang
    );

    // Divisions for number of degrees in the range
    const int t_range = app_state->temp_high - app_state->temp_low;
    const int ang_s = end_ang - start_ang;

    if (t_range > 1) {
      graphics_context_set_fill_color(ctx, GColorDarkGray);

      for (int i = 1; i < t_range; i++) {
        const int item_ang = start_ang + (i * ang_s) / t_range;

        graphics_fill_radial(
          ctx,
          blk_arc_r,
          GOvalScaleModeFitCircle,
          inner_w,
          item_ang - sep_ang,
          item_ang + sep_ang
        );
      }
    }

    // Position of current temperature on the scale
    if (app_state->temp_high < app_state->temp_low) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "High temp below low temp");
    } else {
      // Stay within scale
      int temp = app_state->current_temp;
      if (temp < app_state->temp_low) {
        temp = app_state->temp_low;
      }
      if (temp > app_state->temp_high) {
        temp = app_state->temp_high;
      }
      const int dot_ang = start_ang + (((temp - app_state->temp_low) * ang_s) / t_range) + (item_l / 2);
      draw_gauge_dot(ctx, center, dot_ang);
    }

    graphics_context_set_antialiased(ctx, true);
  }

  graphics_context_set_antialiased(ctx, false);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_antialiased(ctx, false);

  PersistData *persist_data = data_get_persist_data();

  GRect bounds = layer_get_bounds(layer);
  const GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  // Progress through the day
#ifdef TEST
  t->tm_hour = 12;
  t->tm_min = 0;
#endif
  const int minutes = (t->tm_hour * 60) + t->tm_min;
  s_day_prog_ang = (TRIG_MAX_ANGLE * minutes) / MINUTES_PER_DAY;
  // Adjust angle so noon is at the top, not middnight
  s_day_prog_ang = (s_day_prog_ang - (2 * (TRIG_MAX_ANGLE / 4))) % TRIG_MAX_ANGLE;

  draw_sunrise_and_sunset_ring(ctx, bounds, center);
  draw_decorations(ctx, bounds, center);
  draw_weather(ctx, bounds);
  draw_center_post(ctx, center);
  draw_date_and_time_display(ctx, bounds, t, center);

  // Complications!
  draw_arc_complication(ctx, bounds, center, 0, persist_data->slot_1);
  draw_arc_complication(ctx, bounds, center, 1, persist_data->slot_2);
  draw_arc_complication(ctx, bounds, center, 2, persist_data->slot_3);
  draw_arc_complication(ctx, bounds, center, 3, persist_data->slot_4);

  // Tapped state
  if (s_tapped) {
    // TODO: Show full time
  }
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
  if (tick_time->tm_min == 0 && tick_time->tm_sec == 0) {
    comm_request_weather();
  }

  layer_mark_dirty(s_canvas_layer);
}

static void bt_handler(bool connected) {
  s_connected = connected;
  if (!connected) vibes_double_pulse();

  layer_mark_dirty(s_canvas_layer);
}

#ifdef PBL_HEALTH
static void update_step_count() {
  HealthMetric metric = HealthMetricStepCount;
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(
    metric,
    time_start_of_today(),
    time(NULL)
  );

  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_steps = (int)health_service_sum_today(metric);
  } else {
    s_steps = 0;
  }
}

static void health_handler(HealthEventType event, void *context) {
  if (event != HealthEventMovementUpdate) return;

  update_step_count();
}
#endif

/********************************************* Window *********************************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_BLANK);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  gbitmap_destroy(s_weather_bitmap);

  layer_destroy(s_canvas_layer);

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

  main_window_reload();
}

// Weather data or config came in
void main_window_reload() {
  if (s_weather_bitmap) {
    gbitmap_destroy(s_weather_bitmap);
    s_weather_bitmap = NULL;
  }
  s_weather_bitmap = gbitmap_create_with_resource(data_get_weather_icon_id());

#ifdef PBL_HEALTH
  health_service_events_unsubscribe();
  if (data_is_health_used()) {
    if (!health_service_events_subscribe(health_handler, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
    }
    update_step_count();
  }
#endif

  layer_mark_dirty(s_canvas_layer);
}
