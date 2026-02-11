#include "log_window.h"

#define GRAPH_MARGIN scl_x(65)
#define GRAPH_W scl_x_pp({.o = 890, .e = 880})
#define GRAPH_H scl_y(550)
#define NOTCH_S scl_x(35)

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
#define POINT_S 3
#else
#define POINT_S 2
#endif

static Window *s_window;
static TextLayer *s_header_layer, *s_desc_layer;
static Layer *s_canvas_layer;

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

////////////////////////////////////////////// Layout //////////////////////////////////////////////

static bool graph_is_available() {
  return data_get_log_length() >= MIN_SAMPLES_FOR_GRAPH;
}

static char* get_exp_sign(int acc) {
  if (acc == 0) return "=";
  return acc > 0 ? ">" : "<";
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Button hints
  util_draw_button_hints(ctx, (bool[3]){false, true, false});

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_text_color(ctx, GColorBlack);

  // Title underline
  graphics_fill_rect(
    ctx,
    GRect(0, scl_y(110), PS_DISP_W - ACTION_BAR_W, LINE_W),
    0,
    GCornerNone
  );

  const int log_len = data_get_log_length();
#ifdef FEATURE_ANIMATIONS
  int count = util_is_animating() ? s_anim_count : log_len;
#else
  int count = log_len;
#endif

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

  const int x_gap = GRAPH_W / count;
  const int root_y = scl_y(160);
  // Always show full X axis size, even when animating
  const int max_x = GRAPH_W;

  // Draw Y and X axes
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    GRect(GRAPH_MARGIN, root_y, LINE_W, GRAPH_H),
    0,
    GCornerNone
  );
  graphics_fill_rect(
    ctx,
    GRect(GRAPH_MARGIN, root_y + GRAPH_H, max_x - GRAPH_MARGIN + LINE_W, LINE_W),
    0,
    GCornerNone
  );

  // Draw Y axis notches every 10 units within the range
  int y_range = high_v - low_v;
  if (y_range == 0) y_range = 10;
  for (int y_val = low_v; y_val <= high_v; y_val += 10) {
    const int notch_y = root_y + GRAPH_H - ((y_val - low_v) * GRAPH_H / y_range);
    graphics_fill_rect(
      ctx,
      GRect(GRAPH_MARGIN - NOTCH_S, notch_y - LINE_W / 2, NOTCH_S, LINE_W),
      0,
      GCornerNone
    );
  }

  // Draw X axis notches at x_gap intervals, including last
  for (int i = 0; i < count; i++) {
    if (i % 2 == 0 || i == count - 1) {
      const int notch_x = GRAPH_MARGIN + (i * x_gap);
      graphics_fill_rect(
        ctx,
        GRect(notch_x - LINE_W / 2, root_y + GRAPH_H, LINE_W, NOTCH_S),
        0,
        GCornerNone
      );
    }
  }

  // No data?
  if (!graph_is_available()) {
    graphics_draw_text(
      ctx,
      "Not enough data (yet!)",
      scl_get_font(SFI_Medium),
      GRect(20, scl_y(180), PS_DISP_W - ACTION_BAR_W - 40, 300),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      NULL
    );
    return;
  }

  const bool flip = count - s_selection < (count / 2);
  int prev_x = -1, prev_y = -1;

  graphics_context_set_stroke_width(ctx, 1);

  // Draw points from oldest to newest
  for (int i = 0; i < count; i++) {
    const int idx = count - i - 1;
    const Sample *s = data_get_sample(idx);
    if (!s || !util_is_not_status(s->charge_perc)) continue;

    const int x = GRAPH_MARGIN + (i * x_gap);
    const int y = root_y + GRAPH_H - (( (s->charge_perc - low_v) * GRAPH_H) / y_range);

    // Draw this point
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(x, y), POINT_S);

    // If selected point, show dashed lines
    if (idx == s_selection) {
      if (!flip) {
        for (int lx = GRAPH_MARGIN; lx <= x; lx += 8) {
          graphics_draw_line(ctx, GPoint(lx, y), GPoint(lx + 3, y));
        }
      } else {
        for (int lx = x; lx <= max_x; lx += 8) {
          graphics_draw_line(ctx, GPoint(lx, y), GPoint(lx + 3, y));
        }
      }
      for (int ly = y; ly <= root_y + GRAPH_H; ly += 8) {
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
        const int est_y = root_y + GRAPH_H - (((est_val - low_v) * GRAPH_H) / y_range);

        if (est_y != y) {
          // Draw prediction circle
          graphics_draw_circle(ctx, GPoint(x, est_y), POINT_S);
        }
      }
    }
  }

  // Box around date
  const int box_h = scl_y(100);
  const int date_box_y = root_y + GRAPH_H + scl_y(60);
  const GRect box_rect = scl_center_x(
    GRect(0, date_box_y, scl_x_pp({.o = 630, .e = 580}), box_h)
  );
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, box_rect);

  // Draw 'DD MMM' box of selected point
  Sample *sel_s = data_get_sample(s_selection);
  if (sel_s && util_is_not_status(sel_s->timestamp)) {
    time_t ts = sel_s->timestamp;

    static char s_date_buff[18];
    struct tm *time_info = localtime(&ts);
    strftime(s_date_buff, sizeof(s_date_buff), "%d %b - %H:%M", time_info);

    graphics_draw_text(
      ctx,
      s_date_buff,
      scl_get_font(SFI_Small),
      GRect(0, date_box_y - scl_y(25), PS_DISP_W, 300),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
    );
  }

  // Draw sample's value in box, on either side
  if (sel_s && util_is_not_status(sel_s->charge_perc)) {
    const int sel_y = root_y + GRAPH_H - (((sel_s->charge_perc - low_v) * GRAPH_H) / y_range);
    const int box_w = scl_x(210);
    const int box_x = flip ? PS_DISP_W - box_w : 0;
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
      GRect(box_x, box_y - scl_y(25), box_w, box_h),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
    );
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  const int count = data_get_log_length();
  if (s_selection < count - 1) s_selection++;

  layer_mark_dirty(s_canvas_layer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  log_window_push(s_selection);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_selection > 0) s_selection--;

  layer_mark_dirty(s_canvas_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_header_layer = util_make_text_layer(
    GRect(0, scl_y_pp({-30, .e = -25}), PS_DISP_W - ACTION_BAR_W, 100),
    scl_get_font(SFI_Small)
  );
  text_layer_set_text(s_header_layer, "Change over time");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_header_layer));

  s_canvas_layer = layer_create(
    GRect(0, 0, PS_DISP_W, bounds.size.h)
  );
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_desc_layer = util_make_text_layer(
    GRect(scl_x(50), scl_y(860), PS_DISP_W, 100),
    scl_get_font(SFI_Small)
  );
  text_layer_set_overflow_mode(s_desc_layer, GTextOverflowModeWordWrap);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));;

  if (graph_is_available()) {
    const int acc = data_calculate_accuracy();
    static char s_desc_buff[48];
    snprintf(
      s_desc_buff,
      sizeof(s_desc_buff),
#if defined(PBL_PLATFORM_EMERY)
      "Trend: %s%d%% (%s expected)",
#else
      "Trend: %s%d%% (%s expd.)",
#endif
      acc >= 0 ? "+" : "",
      acc,
      get_exp_sign(acc)
    );
    text_layer_set_text(s_desc_layer, s_desc_buff);
  }
}

static void window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_desc_layer);
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
