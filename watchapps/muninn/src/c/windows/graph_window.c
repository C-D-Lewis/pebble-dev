#include "graph_window.h"

#define ROOT_Y scl_y_pp({.o = 110, .c = 200, .e = 115, .g = 200})
#define GRAPH_MARGIN scl_x_pp({.o = 25, .c = 130, .g = 130})
#define GRAPH_W scl_x_pp({.o = 930, .c = 760, .g = 760})
#define GRAPH_H scl_y_pp({.o = 480, .c = 400, .e = 490, .g = 430})
#define NOTCH_S scl_x(25)
#define LOG_Y scl_y_pp({.o = 615, .c = 630, .e = 635, .g = 650})
#define LOG_X_START scl_x_pp({.o = 20, .c = 70, .g = 80})

// Not scaled
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
  #define POINT_S 3
  #define DIV_H 3
#else
  #define POINT_S 2
  #define DIV_H 2
#endif

static Window *s_window;
static Layer *s_canvas_layer, *s_header_layer;

static int s_selection = 0;
#ifdef FEATURE_ANIMATIONS
static int s_anim_count = 0;
#endif

//////////////////////////////////////////// Animations ////////////////////////////////////////////

#ifdef FEATURE_ANIMATIONS
static void anim_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_count = util_anim_percentage(dist_normalized, data_get_log_length());

  layer_mark_dirty(s_canvas_layer);
}
#endif

////////////////////////////////////////////// Drawing /////////////////////////////////////////////

static void draw_datetime(GContext *ctx, const GRect bounds, const Sample *s) {
  time_t ts_time = s->timestamp;
  const struct tm *ts_info = localtime(&ts_time);
  static char s_datetime_buff[16];
  strftime(s_datetime_buff, sizeof(s_datetime_buff), "%b %d - %H:%M", ts_info);
  graphics_draw_text(
    ctx,
    s_datetime_buff,
    scl_get_font(SFI_Small),
    GRect(LOG_X_START, LOG_Y - scl_y(5), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  const int log_len = data_get_log_length();
  static char s_pos_buff[8];
  const int display_pos = log_len - s_selection;
  snprintf(s_pos_buff, sizeof(s_pos_buff), "%d/%d", display_pos, log_len);
  graphics_draw_text(
    ctx,
    s_pos_buff,
    scl_get_font(SFI_Small),
    GRect(LOG_X_START, LOG_Y - scl_y(5), PS_DISP_W - PBL_IF_ROUND_ELSE(40, 8), 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
}

static void draw_log_detail(GContext *ctx, const GRect bounds, const Sample *s) {
  const int hours = (s->time_diff + 1800) / 3600;
  static char s_res_buff[32];

#ifdef PBL_ROUND
  if (s->result == STATUS_NO_CHANGE) {
    snprintf(s_res_buff, sizeof(s_res_buff), "No change (%dh)", hours);
  } else if (s->result == STATUS_CHARGED) {
    snprintf(s_res_buff, sizeof(s_res_buff), "Charged %d%% (%dh)", -(s->charge_diff), hours);
  } else {
    snprintf(s_res_buff, sizeof(s_res_buff), "Drained %d%% (%dh)", s->charge_diff, hours);
  }
#else
  if (s->result == STATUS_NO_CHANGE) {
    snprintf(s_res_buff, sizeof(s_res_buff), "No change in %d hrs", hours);
  } else if (s->result == STATUS_CHARGED) {
    snprintf(s_res_buff, sizeof(s_res_buff), "Charged %d%% in %d hrs", -(s->charge_diff), hours);
  } else {
    snprintf(s_res_buff, sizeof(s_res_buff), "Drained %d%% in %d hrs", s->charge_diff, hours);
  }
#endif

  graphics_draw_text(
    ctx,
    s_res_buff,
    scl_get_font(SFI_Medium),
    GRect(
      LOG_X_START,
      LOG_Y + scl_y_pp({.o = 80, .c = 70, .e = 90, .g = 90}),
      PS_DISP_W - LOG_X_START,
      200
    ),
    GTextOverflowModeWordWrap,
#ifdef PBL_ROUND
    GTextAlignmentCenter,
#else
    GTextAlignmentLeft,
#endif
    NULL
  );

#ifndef PBL_ROUND
  // Divider
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    GRect(0, LOG_Y + scl_y_pp({.o = 260, .e = 250, .g = 190}), PS_DISP_W, 1),
    0,
    GCornerNone
  );
#endif

  // Drain since 24h ago
  time_t now = time(NULL);
  Sample *last_s = data_get_sample(0);
  if (!last_s) return;

  Sample *s_24h = NULL;
  for (int i = 0; i < data_get_log_length(); i++) {
    Sample *find_s = data_get_sample(i);
    if (!find_s) break;
    if (find_s->timestamp <= now - SECONDS_PER_DAY) {
      s_24h = find_s;
      break;
    }
  }
  if (!s_24h) return;

  static char s_24h_buff[25];
  snprintf(
    s_24h_buff,
    sizeof(s_24h_buff),
    "24h change: %d%%",
    last_s->charge_perc - s_24h->charge_perc
  );
  graphics_draw_text(
    ctx,
    s_24h_buff,
    scl_get_font(SFI_Small),
    GRect(
      LOG_X_START,
      LOG_Y + scl_y_pp({.o = 250, .c = 190, .e = 235, .g = 190}),
      PS_DISP_W - (2 * LOG_X_START),
      200
    ),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

////////////////////////////////////////////// Layout //////////////////////////////////////////////

static bool graph_is_available() {
  return data_get_log_length() >= MIN_SAMPLES_FOR_GRAPH;
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_text_color(ctx, GColorBlack);

  const int log_len = data_get_log_length();
#ifdef FEATURE_ANIMATIONS
  int count = util_is_animating() ? s_anim_count : log_len;
#else
  int count = log_len;
#endif
  if (count == 0) count = 1;

  // Find Y ranges, rounded to nearest 10
  int low_v = 100, high_v = 0;
  for (int i = 0; i < count; i++) {
    const Sample *vs = data_get_sample(i);
    if (!vs) break; // No data yet, but we still want to render the skeleton
    if (vs->charge_perc < low_v) {
      low_v = vs->charge_perc;
    }
    if (vs->charge_perc > high_v) {
      high_v = vs->charge_perc;
    }
  }
  low_v = ((low_v / 10) * 10) - 5;
  high_v = (((high_v + 9) / 10) * 10) + 5;

  // Dummy values for illustrative purposes
  if (!graph_is_available()) {
    low_v = 50;
    high_v = 80;
    count = NUM_SAMPLES;
  }

  // Find total time and so the gap between intervals
  const Sample *newest_s = data_get_sample(0);
  const Sample *oldest_s = data_get_sample(count - 1);
  long total_time = (newest_s && oldest_s)
    ? (newest_s->timestamp - oldest_s->timestamp)
    : SECONDS_PER_DAY;
  if (total_time <= 0) total_time = SECONDS_PER_DAY;

  // Draw Y and X axes
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(GRAPH_MARGIN, ROOT_Y, LINE_W, GRAPH_H), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(GRAPH_MARGIN, ROOT_Y + GRAPH_H, GRAPH_W, LINE_W), 0, GCornerNone);

  // Draw Y axis notches every 10 units within the range
  int y_range = high_v - low_v;
  if (y_range == 0) y_range = 10;
  for (int y_val = low_v; y_val <= high_v; y_val += 10) {
    const int notch_y = ROOT_Y + GRAPH_H - ((y_val - low_v) * GRAPH_H / y_range);
    graphics_fill_rect(
      ctx,
      GRect(GRAPH_MARGIN - NOTCH_S, notch_y - LINE_W / 2, NOTCH_S, LINE_W),
      0,
      GCornerNone
    );
  }

  // No data?
  if (!graph_is_available()) {
    graphics_draw_text(
      ctx,
      "Not enough data\n(yet!)",
      scl_get_font(SFI_Medium),
      GRect(0, scl_y(220), PS_DISP_W, 300),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL
    );

    static char s_no_data_buff[28];
    const int remaining = MIN_SAMPLES_FOR_GRAPH - data_get_log_length();
    snprintf(
      s_no_data_buff,
      sizeof(s_no_data_buff),
      "Waiting for %d more samples",
      remaining
    );
    graphics_draw_text(
      ctx,
      s_no_data_buff,
      scl_get_font(SFI_Medium),
      GRect(0, LOG_Y + scl_y(30), PS_DISP_W, 150),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
    );
    return;
  }

  const int box_w = scl_x_pp({.o = 210, .c = 170, .g = 150});
  const bool flip = count - s_selection < (count / 2);
  int prev_x = -1, prev_y = -1;

  // Draw points from oldest to newest
  for (int i = 0; i < count; i++) {
    const int idx = count - i - 1;
    const Sample *s = data_get_sample(idx);
    if (!s || !util_is_not_status(s->charge_perc)) continue;

    if (!oldest_s) continue;
    const int oldest_ts = oldest_s->timestamp;

    // Find it's percentage x of the total based on timestamp more than oldest ts
    const int x = GRAPH_MARGIN + (((s->timestamp - oldest_ts) * GRAPH_W) / total_time);
    const int y = ROOT_Y + GRAPH_H - (( (s->charge_perc - low_v) * GRAPH_H) / y_range);

    // Draw this point
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(x, y), POINT_S);

    // Draw its notch on the X axis
    time_t ts = s->timestamp;
    struct tm *s_time = localtime(&ts);
    const int x_notch_mod = log_len < 8 ? 6 : 12;
    if (s_time->tm_hour % x_notch_mod == 0) {
      graphics_fill_rect(
        ctx,
        GRect(x - (LINE_W / 2), ROOT_Y + GRAPH_H + LINE_W, LINE_W, NOTCH_S),
        0,
        GCornerNone
      );
    }

    // If selected point, show dashed lines
    graphics_context_set_stroke_width(ctx, 1);
    if (idx == s_selection) {
      if (!flip) {
        for (int lx = GRAPH_MARGIN; lx <= x; lx += 8) {
          graphics_draw_line(ctx, GPoint(lx, y), GPoint(lx + 3, y));
        }
      } else {
        for (int lx = x; lx <= GRAPH_W + box_w; lx += 8) {
          graphics_draw_line(ctx, GPoint(lx, y), GPoint(lx + 3, y));
        }
      }
      for (int ly = y; ly <= ROOT_Y + GRAPH_H; ly += 8) {
        graphics_draw_line(ctx, GPoint(x, ly), GPoint(x, ly + 3));
      }
    }

    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, GColorBlack);

    if (idx + 1 <= count) {
      // The next oldest sample is idx + 1
      const Sample *prev_s = data_get_sample(idx + 1);
      if (!prev_s) break;

      // Draw the line to this point
      if (prev_x != -1 && util_is_not_status(prev_s->charge_perc)) {
        graphics_draw_line(ctx, GPoint(prev_x, prev_y), GPoint(x, y));
      }
      prev_x = x;
      prev_y = y;

      // Draw prediction based on previous rate
      if (util_is_not_status(prev_s->rate)) {
        const int time_diff = s->timestamp - prev_s->timestamp;
        const int est_drop = (prev_s->rate * time_diff) / SECONDS_PER_DAY;
        const int est_val = prev_s->charge_perc - est_drop;
        const int est_y = ROOT_Y + GRAPH_H - (((est_val - low_v) * GRAPH_H) / y_range);

        if (est_y != y && s->result != STATUS_CHARGED && est_y < ROOT_Y + GRAPH_H) {
          // Draw prediction circle
          graphics_draw_circle(ctx, GPoint(x, est_y), POINT_S);
        }
      }
    }
  }

  // Draw sample's value in box, on either side
  Sample *sel_s = data_get_sample(s_selection);
  if (sel_s && util_is_not_status(sel_s->charge_perc)) {
    const int sel_y = ROOT_Y + GRAPH_H - (((sel_s->charge_perc - low_v) * GRAPH_H) / y_range);
    const int box_h = scl_y(100);
#ifdef PBL_ROUND
    const int box_x = flip ? PS_DISP_W - ((3 * box_w) / 2) : (box_w / 2);
#else
    const int box_x = flip ? PS_DISP_W - box_w : 0;
#endif
    const int box_y = sel_y - (box_h / 2);
    const GRect box_rect = GRect(box_x, box_y, box_w, box_h);

    // Draw value box
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, box_rect, 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_rect(ctx, box_rect);

    static char s_value_buff[6];
    snprintf(
      s_value_buff,
      sizeof(s_value_buff),
      sel_s->charge_perc != 100 ? "%d%%" : "%d",
      sel_s->charge_perc
    );
    graphics_draw_text(
      ctx,
      s_value_buff,
      scl_get_font(SFI_Small),
      GRect(box_x, box_y - scl_y_pp({.o = 25, .c = 15, .g = 15}), box_w, box_h),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
    );
  }

#ifndef PBL_ROUND
  // Divider
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, LOG_Y, PS_DISP_W, DIV_H), 0, GCornerNone);
#endif

  // Log details
  draw_datetime(ctx, bounds, sel_s);
  draw_log_detail(ctx, bounds, sel_s);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  const int count = data_get_log_length();
  if (s_selection < count - 1) s_selection++;

  layer_mark_dirty(s_canvas_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_selection > 0) s_selection--;

  layer_mark_dirty(s_canvas_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  s_header_layer = util_create_header_layer(PBL_IF_ROUND_ELSE("Graph", "Change over time"), 18);
  layer_add_child(root_layer, s_header_layer);

  s_canvas_layer = layer_create(GRect(0, 0, PS_DISP_W, PS_DISP_H));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_header_layer);
  layer_destroy(s_canvas_layer);

  window_destroy(window);
  s_window = NULL;
}

static void window_disappear(Window *window) {
#ifdef FEATURE_ANIMATIONS
  util_stop_animation();
#endif
}

void graph_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload,
      .disappear = window_disappear
    });
  }

  window_stack_push(s_window, true);

#ifdef FEATURE_ANIMATIONS
  if (graph_is_available()) {
    static AnimationImplementation anim_implementation = { .update = anim_update };
    util_animate(500, 0, &anim_implementation, true);
  }
#endif
}
