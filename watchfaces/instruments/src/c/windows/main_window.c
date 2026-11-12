#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;
static GBitmap *s_weather_bitmap;

static AppTimer *s_tap_timer;
static bool s_tapped = false, s_connected;
static int s_day_prog_ang, s_steps;

/******************************************** Drawing *********************************************/

static void draw_text_bubble(GContext *ctx, GRect bounds, char *text, GTextAlignment alignment, GFont font) {
  const int corner_r = 5;
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, corner_r, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, grect_inset(bounds, GEdgeInsets(OUTER_RING_W)), corner_r, GCornersAll);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    text,
    font,
    grect_inset(bounds, GEdgeInsets2(0, 2 * OUTER_RING_W)),
    GTextOverflowModeWordWrap,
    alignment,
    NULL
  );
}

static void draw_date_and_time_display(GContext *ctx, GRect bounds, struct tm *t, GPoint center) {
  if (!s_tapped) {
    // Day of week
    static char dow_buff[8];
    strftime(dow_buff, sizeof(dow_buff), "%a", t);
    graphics_context_set_text_color(ctx, GColorLightGray);
    graphics_draw_text(
      ctx,
      dow_buff,
      scl_get_font(SFI_Date),
      GRect(scl_x(610), scl_y(360), 30, bounds.size.h),
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
      GRect(scl_x(610), scl_y(430), 30, 100),
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
      GRect(scl_x(610), scl_y(545), 30, 100),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL
    );
  }

  // Day progress hand
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

  if (s_tapped) {
    // Show time over center post
    static char time_buff[8];
    strftime(time_buff, sizeof(time_buff), "%H:%M", t);
    draw_text_bubble(
      ctx,
      GRect(scl_x(380), scl_y(430), scl_x(255), scl_y(145)),
      time_buff,
      GTextAlignmentCenter,
      scl_get_font(SFI_DateLarge)
    );
  }
}

static void draw_weather(GContext *ctx, GRect bounds) {
  if (s_tapped) return;

  AppState *app_state = data_get_app_state();

  // Weather icon
  graphics_draw_bitmap_in_rect(
    ctx,
    s_weather_bitmap,
    GRect(scl_x(240), scl_y(450), ICON_SIZE, ICON_SIZE)
  );

  // Temperature
  static char temp_buff[8];
  if (app_state->current_code == DATA_EMPTY) {
    snprintf(temp_buff, sizeof(temp_buff), " ");
  } else if (app_state->current_code == DATA_ERROR) {
    snprintf(temp_buff, sizeof(temp_buff), "ERR");
  } else {
    snprintf(temp_buff, sizeof(temp_buff), "%d", util_convert_temp(app_state->current_temp));
  }
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(
    ctx,
    temp_buff,
    scl_get_font(SFI_Weather),
    GRect(scl_x(355), scl_y(450), 30, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );
}

static void draw_decorations(GContext *ctx, GRect bounds, GPoint center) {
  // Day arcs
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET)),
    GOvalScaleModeFitCircle,
    0,
    TRIG_MAX_ANGLE
  );
  graphics_draw_arc(
    ctx,
    grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET + HOUR_DOT_RING_W + OUTER_RING_W / 2)),
    GOvalScaleModeFitCircle,
    0,
    TRIG_MAX_ANGLE
  );

  // Straight line representing midnight and ligher one for mid-day
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 2);
  const int eod_ang = DEG_TO_TRIGANGLE(180);
  const int hand_r = center.x - OUTER_RING_INSET - HOUR_DOT_RING_W - OUTER_RING_W;
  GPoint eod_hand_start = util_make_hand_point(eod_ang, TRIG_MAX_ANGLE, hand_r, center);
  GPoint eod_hand_end = util_make_hand_point(eod_ang, TRIG_MAX_ANGLE, hand_r - 10, center);
  graphics_draw_line(ctx, eod_hand_start, eod_hand_end);
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  GPoint mod_hand_start = util_make_hand_point(0, TRIG_MAX_ANGLE, hand_r, center);
  GPoint mod_hand_end = util_make_hand_point(0, TRIG_MAX_ANGLE, hand_r - 10, center);
  graphics_draw_line(ctx, mod_hand_start, mod_hand_end);

  // Radial dots for tracking hours
  const int dot_r = center.x - OUTER_RING_INSET - (HOUR_DOT_RING_W / 2) - 1;
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i < 24; i += 1) {
    if (i % 3 == 0) {
      // Every 3 hours
      GPoint dot_point = util_make_hand_point(i, 24, dot_r, center);
      graphics_context_set_fill_color(ctx, GColorLightGray);
      graphics_fill_circle(ctx, dot_point, HOUR_DOT_S);
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_circle(ctx, dot_point, HOUR_DOT_S / 2);
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
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_circle(ctx, center, CENTER_POST_W + 2);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, CENTER_POST_W);
}

static void draw_sunrise_and_sunset_ring(GContext *ctx, GRect bounds, GPoint center) {
  AppState *app_state = data_get_app_state();

  if (!data_is_weather_valid()) return;

  const GRect arc_r = grect_inset(bounds, GEdgeInsets(OUTER_RING_INSET));

  int s_rise_ang = util_get_time_angle(app_state->sunrise);
  int s_set_ang = util_get_time_angle(app_state->sunset);
  // Similar adjustment to angle so noon is at the top, not midnight
  s_rise_ang = (s_rise_ang - (2 * (TRIG_MAX_ANGLE / 4))) % TRIG_MAX_ANGLE;
  s_set_ang = (s_set_ang - (2 * (TRIG_MAX_ANGLE / 4))) % TRIG_MAX_ANGLE;

  // Day and night arc sections
  graphics_context_set_fill_color(ctx, s_tapped ? GColorDarkGray : data_get_theme_day_color());
  graphics_fill_radial(ctx, arc_r, GOvalScaleModeFitCircle, HOUR_DOT_RING_W, s_rise_ang, s_set_ang);
  graphics_context_set_fill_color(ctx, s_tapped ? GColorDarkGray : data_get_theme_night_color());
  if (s_rise_ang < s_set_ang) {
    s_rise_ang += TRIG_MAX_ANGLE;
  }
  graphics_fill_radial(ctx, arc_r, GOvalScaleModeFitCircle, HOUR_DOT_RING_W, s_set_ang, s_rise_ang);

  // Dawn notch on each border between sunrise and sunset
  const int notch_ang = TRIG_MAX_ANGLE / 256;
  graphics_context_set_fill_color(ctx, s_tapped ? GColorDarkGray : GColorChromeYellow);
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
}

static void draw_gauge_dot(GContext *ctx, GPoint center, int angle, GColor color) {
  // Due to how dots are plotted, needs different offset in different positions
  const int offset = (angle < (3 * TRIG_MAX_ANGLE) / 4) ? 3 : 2;

  GPoint dot_point = util_make_hand_point(
    angle,
    TRIG_MAX_ANGLE,
    center.x - (COMPLICATION_W / 2) - offset,
    center
  );
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_circle(ctx, dot_point, GAUGE_DOT_S);
}

static void draw_empty_complication(GContext *ctx, GRect arc_r, int start_ang, int end_ang) {
  graphics_context_set_stroke_width(ctx, OUTER_RING_W);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
}

static void draw_empty_gauge(GContext *ctx, GRect arc_r, int start_ang, int end_ang, int inner_w) {
  // Outline and black fill for empty gauge
  graphics_context_set_stroke_width(ctx, COMPLICATION_W);
  graphics_context_set_stroke_color(ctx, COLOR_COMPLICATION);
  graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
  graphics_context_set_stroke_width(ctx, inner_w);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
}

static void draw_arc_complication(GContext *ctx, GRect bounds, GPoint center, int slot, char *type) {
  AppState *app_state = data_get_app_state();
  PersistData *persist_data = data_get_persist_data();

  const bool is_next_rain = strcmp(type, COMP_NEXT_RAIN) == 0;
  const bool is_next_weather = strcmp(type, COMP_NEXT_WEATHER) == 0;
  const bool is_battery = strcmp(type, COMP_BATTERY) == 0;
  const bool is_phone_battery = strcmp(type, COMP_PHONE_BATTERY) == 0;
  const bool is_month_prog = strcmp(type, COMP_MONTH_PROGRESS) == 0;
  const bool is_year_prog = strcmp(type, COMP_YEAR_PROGRESS) == 0;
  const bool is_moon_phase = strcmp(type, COMP_MOON_PHASE) == 0;

  const int slot_count = data_get_complication_count();
  const int gap_deg = slot_count > 4 ? 20 : 40;
  const int gap_ang = DEG_TO_TRIGANGLE(gap_deg);
  const int slot_l_deg = (360 - (gap_deg * slot_count)) / slot_count;
  const int slot_l_ang = DEG_TO_TRIGANGLE(slot_l_deg);
  // Slots are 0-3, clockwise from top
  const int start_ang = (gap_ang / 2) + (slot * slot_l_ang) + (slot * gap_ang);
  const int end_ang = start_ang + slot_l_ang;
  const GTextAlignment alignment = GTextAlignmentCenter;

  // Place the label at the arc midpoint
  GRect txt_r = GRect(0, 0, scl_x(250), scl_y(100));
  const int origin_ang = start_ang + slot_l_ang / 2;
  GPoint txt_p = util_make_hand_point(
    origin_ang,
    TRIG_MAX_ANGLE,
    (PS_DISP_W / 2) - scl_x(13),
    center
  );
  // Slightly longer
  if (is_next_weather) {
    txt_r.size.w += scl_x(70);
  }

  // Put the label box inward from the anchor, toward the display center.
  const int dx = txt_p.x - center.x;
  if (abs(dx) < txt_r.size.w / 2) {
    txt_r.origin.x = txt_p.x - txt_r.size.w / 2;
  } else {
    txt_r.origin.x = dx < 0 ? txt_p.x : txt_p.x - txt_r.size.w;
  }
  txt_r.origin.y = txt_p.y < center.y ? txt_p.y : txt_p.y - (txt_r.size.h * 2) / 3;
  if (txt_r.origin.y < 0) {
    txt_r.origin.y = 0;
  } else if (txt_r.origin.y + txt_r.size.h > PS_DISP_H) {
    txt_r.origin.y = PS_DISP_H - txt_r.size.h;
  }

  const GRect arc_r = grect_inset(bounds, GEdgeInsets((COMPLICATION_W / 2) + 3));
  const int inner_w = COMPLICATION_W - (OUTER_RING_W + 3);
  const int sep_ang = 75;
  const GRect blk_arc_r = grect_inset(bounds, GEdgeInsets(2 * OUTER_RING_W));
  const int fc_item_l = (slot_l_deg * TRIG_MAX_ANGLE) / (360 * NUM_FORECAST_ITEMS);

  if (strcmp(type, COMP_NONE) == 0) {
    // Just a slim line if no complication in this slot
    draw_empty_complication(ctx, arc_r, start_ang, end_ang);
    return;
  }

  // Fill based on battery/phone battery level
  if (is_battery || is_phone_battery) {
    if (
      is_phone_battery && (
        app_state->phone_battery == DATA_EMPTY || app_state->phone_battery == DATA_ERROR
      )
    ) {
      draw_empty_complication(ctx, arc_r, start_ang, end_ang);
      return;
    }

    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);
  
    int percent = 0;
    if (is_battery) {
      BatteryChargeState charge_state = battery_state_service_peek();
      percent = charge_state.charge_percent;
    } else if (is_phone_battery) {
      percent = app_state->phone_battery;
    }
    const int batt_ang = ((end_ang - start_ang) * percent) / 100;
    graphics_context_set_stroke_color(ctx, is_battery ? COLOR_BATTERY : COLOR_PHONE_BATTERY);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + batt_ang);

    draw_gauge_dot(ctx, center, start_ang + batt_ang, GColorWhite);

    if (s_tapped) {
      static char batt_buff[16];
      if (is_battery) {
        BatteryChargeState charge_state = battery_state_service_peek();
        snprintf(batt_buff, sizeof(batt_buff), "%d%%", charge_state.charge_percent);
      } else if (is_phone_battery) {
        snprintf(batt_buff, sizeof(batt_buff), "P: %d%%", app_state->phone_battery);
      }
      draw_text_bubble(ctx, txt_r, batt_buff, alignment, scl_get_font(SFI_Label));
    }
    return;
  }
  
  // Fill based on progress to step goal
  if (strcmp(type, COMP_STEP_COUNT) == 0) {
    if (s_steps == DATA_EMPTY) {
      draw_empty_complication(ctx, arc_r, start_ang, end_ang);
      return;
    }
    
    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);

    const int step_goal = atoi(persist_data->step_goal);
    const int max_steps = s_steps > step_goal ? step_goal : s_steps;
    const int step_ang = ((end_ang - start_ang) * max_steps) / step_goal;
    graphics_context_set_stroke_color(ctx, COLOR_STEP_COUNT);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + step_ang);

    draw_gauge_dot(ctx, center, start_ang + step_ang, GColorWhite);

    if (s_tapped) {
      static char step_buff[8];
      snprintf(step_buff, sizeof(step_buff), "%d", s_steps);
      draw_text_bubble(ctx, txt_r, step_buff, alignment, scl_get_font(SFI_Label));
    }
    return;
  }

  // Next 6 rain chance or weather codes
  if (is_next_rain || is_next_weather) {
    if (!data_is_weather_valid()) {
      draw_empty_complication(ctx, arc_r, start_ang, end_ang);
      return;
    }

    time_t temp = time(NULL);
    struct tm *t = localtime(&temp);
    const int hours = t->tm_hour;

    int forecast_i = 0;
    if (hours > 24 - NUM_FORECAST_ITEMS) {
      forecast_i = NUM_FORECAST_ITEMS - (24 - hours);
    }

    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);

    // Fill ends - offsets for cleaner drawing
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_context_set_stroke_color(ctx, COLOR_COMPLICATION);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang - 50, end_ang + 50);
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

      const int item_ang = start_ang + (i * fc_item_l);
      const GRect item_r = grect_inset(bounds, GEdgeInsets((2 * OUTER_RING_W) + 1));
      graphics_context_set_fill_color(ctx, color);
      graphics_fill_radial(
        ctx,
        item_r,
        GOvalScaleModeFitCircle,
        inner_w,
        item_ang,
        item_ang + fc_item_l
      );

      // Separator
      if (i == NUM_FORECAST_ITEMS - 1) break;
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_fill_radial(
        ctx,
        item_r,
        GOvalScaleModeFitCircle,
        inner_w,
        item_ang + fc_item_l - sep_ang,
        item_ang + fc_item_l + sep_ang
      );
    }

    // Be in first item all day until last 6 hours
    const int dot_ang = start_ang + (forecast_i * fc_item_l) + (fc_item_l / 2);
    draw_gauge_dot(ctx, center, dot_ang, GColorWhite);

    if (s_tapped) {
      if (is_next_rain) {
        static char rain_buff[8];
        const int chance = data_get_strarr_value(app_state->precip_arr, forecast_i);
        snprintf(rain_buff, sizeof(rain_buff), "%d%%", chance);
        draw_text_bubble(ctx, txt_r, rain_buff, alignment, scl_get_font(SFI_Label));
      }
      if (is_next_weather) {
        static char weather_buff[8];
        const int code = data_get_strarr_value(app_state->code_arr, forecast_i);
        snprintf(weather_buff, sizeof(weather_buff), "%s", data_get_weather_str(code));
        draw_text_bubble(ctx, txt_r, weather_buff, alignment, scl_get_font(SFI_Label));
      }
    }
    return;
  }

  if (strcmp(type, COMP_TEMP_RANGE) == 0) {
    if (!data_is_weather_valid()) {
      draw_empty_complication(ctx, arc_r, start_ang, end_ang);
      return;
    }

    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);

    // One item for each integer temperature, including both endpoints.
    if (app_state->temp_high < app_state->temp_low) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "High temp below low temp");
      return;
    }

    const int item_count = app_state->temp_high - app_state->temp_low + 1;
    const int gauge_ang = end_ang - start_ang;
    const int item_s = gauge_ang / item_count;

    // Fill ends with cool/hot colors
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_context_set_stroke_color(ctx, COLOR_HIGH_TEMP);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);
    graphics_context_set_stroke_color(ctx, COLOR_LOW_TEMP);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + 1000);

    for (int i = 0; i < item_count; i++) {
      const int item_ang = start_ang + (i * item_s);
      
      // Item black section
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_radial(
        ctx,
        blk_arc_r,
        GOvalScaleModeFitCircle,
        inner_w,
        item_ang,
        item_ang + item_s
      );

      // Separator
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      graphics_fill_radial(
        ctx,
        blk_arc_r,
        GOvalScaleModeFitCircle,
        inner_w,
        item_ang - sep_ang,
        item_ang + sep_ang
      );
    }
    
    // Stay within scale
    int temp = app_state->current_temp;
    if (temp < app_state->temp_low) {
      temp = app_state->temp_low;
    }
    if (temp > app_state->temp_high) {
      temp = app_state->temp_high;
    }
    const int item_index = temp - app_state->temp_low;
    const int dot_ang = start_ang + (item_index * item_s) + (item_s / 2);
    draw_gauge_dot(ctx, center, dot_ang, GColorWhite);

    if (s_tapped) {
      static char range_buff[8];
      snprintf(
        range_buff,
        sizeof(range_buff),
        "%d-%d",
        util_convert_temp(app_state->temp_low),
        util_convert_temp(app_state->temp_high)
      );
      draw_text_bubble(ctx, txt_r, range_buff, alignment, scl_get_font(SFI_Label));
    }
    return;
  }

  if (is_month_prog || is_year_prog) {
    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);

    const int month_progress = util_get_month_progress();
    const int year_progress = util_get_year_progress();

    int ang;
    if (is_year_prog) {
      ang = ((end_ang - start_ang) * year_progress) / 100;
    } else {
      ang = ((end_ang - start_ang) * month_progress) / 100;
    }
    graphics_context_set_stroke_color(ctx, is_month_prog ? COLOR_MONTH_PROGRESS : COLOR_YEAR_PROGRESS);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + ang);

    draw_gauge_dot(ctx, center, start_ang + ang, GColorWhite);

    if (s_tapped) {
      static char y_m_buff[8];
      if (is_year_prog) {
        snprintf(y_m_buff, sizeof(y_m_buff), "%d%%", year_progress);
      } else {
        snprintf(y_m_buff, sizeof(y_m_buff), "%d%%", month_progress);
      }
      draw_text_bubble(ctx, txt_r, y_m_buff, alignment, scl_get_font(SFI_Label));
    }
    return;
  }

  if (is_moon_phase) {
    if (app_state->moon_phase == DATA_EMPTY) {
      draw_empty_complication(ctx, arc_r, start_ang, end_ang);
      return;
    }

    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);

    // TODO: DRY this
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_context_set_stroke_color(ctx, COLOR_COMPLICATION);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, end_ang);

    // Arcs in 7ths, full moon in the middle. Shame there is only two shades of grey
    const int fc_item_l = (end_ang - start_ang) / 7;
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang,
      start_ang + fc_item_l
    );
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + fc_item_l,
      start_ang + (2 * fc_item_l)
    );
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + (2 * fc_item_l),
      start_ang + (3 * fc_item_l)
    );
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + (3 * fc_item_l),
      start_ang + (4 * fc_item_l)
    );
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + (4 * fc_item_l),
      start_ang + (5 * fc_item_l)
    );
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + (5 * fc_item_l),
      start_ang + (6 * fc_item_l)
    );
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_radial(
      ctx,
      blk_arc_r,
      GOvalScaleModeFitCircle,
      inner_w,
      start_ang + (6 * fc_item_l),
      end_ang
    );
    graphics_context_set_antialiased(ctx, true);

    const int phase = app_state->moon_phase;
    const int moon_ang = ((end_ang - start_ang) * phase) / 100;
    const GColor color = phase > 43 && phase < 57 ? GColorBlack : GColorWhite;
    draw_gauge_dot(ctx, center, start_ang + moon_ang, color);

    if (s_tapped) {
      static char moon_buff[8];
      if (phase < 13) {
        snprintf(moon_buff, sizeof(moon_buff), "NEW");
      } else if (phase < 25) {
        snprintf(moon_buff, sizeof(moon_buff), "WAX CR");
      } else if (phase < 38) {
        snprintf(moon_buff, sizeof(moon_buff), "1ST QT");
      } else if (phase < 50) {
        snprintf(moon_buff, sizeof(moon_buff), "WAX GB");
      } else if (phase < 63) {
        snprintf(moon_buff, sizeof(moon_buff), "FULL");
      } else if (phase < 75) {
        snprintf(moon_buff, sizeof(moon_buff), "WAN GB");
      } else if (phase < 88) {
        snprintf(moon_buff, sizeof(moon_buff), "3RD QT");
      } else {
        snprintf(moon_buff, sizeof(moon_buff), "WAN CR");
      }
      draw_text_bubble(ctx, txt_r, moon_buff, alignment, scl_get_font(SFI_Label));
    }
    return;
  }

  if (strcmp(type, COMP_UV_INDEX_MAX) == 0) {
    if (app_state->uv_index_max == DATA_EMPTY) {
      draw_empty_complication(ctx, arc_r, start_ang, end_ang);
      return;
    }
    
    draw_empty_gauge(ctx, arc_r, start_ang, end_ang, inner_w);

    const int uv_index_max = app_state->uv_index_max;
    const int step_ang = ((end_ang - start_ang) * uv_index_max) / MAX_UV_INDEX;
    graphics_context_set_stroke_color(ctx, COLOR_UV_INDEX_MAX);
    graphics_context_set_stroke_width(ctx, inner_w);
    graphics_draw_arc(ctx, arc_r, GOvalScaleModeFitCircle, start_ang, start_ang + step_ang);

    draw_gauge_dot(ctx, center, start_ang + step_ang, GColorWhite);

    if (s_tapped) {
      static char step_buff[8];
      snprintf(step_buff, sizeof(step_buff), "UV: %d", uv_index_max);
      draw_text_bubble(ctx, txt_r, step_buff, alignment, scl_get_font(SFI_Label));
    }
    return;
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  // Progress through the day
#ifdef TEST
  t->tm_hour = 14;
  t->tm_min = 45;
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
  for (int i = 0; i < data_get_complication_count(); i++) {
    char *type = data_get_nth_complication_type(i);
    if (strcmp(type, COMP_NONE) == 0) continue;
    draw_arc_complication(ctx, bounds, center, i, type);
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
    s_steps = DATA_EMPTY;
  }
}
#endif

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  if (
    // On the hour
    (tick_time->tm_min == 0 && tick_time->tm_sec == 0) ||
    // Retry weather every ten minutes
    (tick_time->tm_min % 10 == 0 && tick_time->tm_sec == 0 && !data_is_weather_valid())
  ) {
    comm_request_data();
  }

  update_step_count();

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

  layer_mark_dirty(s_canvas_layer);
}
