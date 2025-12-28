#include "log_window.h"

// TODO: Use pebble-scalable when finishing this off

#if defined(PBL_PLATFORM_EMERY)
  #define TITLE_HEIGHT 28
  #define GRAPH_MARGIN 28
  #define DESC_Y 26
  #define ZERO_OFFSET 18
#else
  #define TITLE_HEIGHT 22
  #define GRAPH_MARGIN 24
  #define DESC_Y 16
  #define ZERO_OFFSET 17
#endif

#define GRAPH_SIZE (DISPLAY_W - (2 * GRAPH_MARGIN))
#define FONT_KEY_M FONT_KEY_GOTHIC_18
#define FONT_KEY_S FONT_KEY_GOTHIC_14

static Window *s_window;
static TextLayer *s_header_layer, *s_desc_layer;
static Layer *s_canvas_layer;

static GFont s_font_s, s_font_m;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  const int count = data_get_log_length();
  const int x_gap = (GRAPH_SIZE + GRAPH_MARGIN) / count;

  const int root_y = TITLE_HEIGHT + GRAPH_MARGIN + 5;
  const int x_axis_width = GRAPH_SIZE + (count * x_gap);

  // Draw Y and X axes
  graphics_draw_line(
    ctx,
    GPoint(GRAPH_MARGIN, root_y),
    GPoint(GRAPH_MARGIN, root_y + GRAPH_SIZE)
  );
  graphics_draw_line(
    ctx,
    GPoint(GRAPH_MARGIN, root_y + GRAPH_SIZE),
    GPoint(GRAPH_MARGIN + x_axis_width, root_y + GRAPH_SIZE)
  );

  // Draw Y axis labels (every 20, 0 to 100)
  graphics_context_set_text_color(ctx, GColorBlack);
  for (int i = 0; i <= 5; i++) {
    const int value = i * 20;
    const int y = root_y + GRAPH_SIZE - ((value * GRAPH_SIZE) / 100);
    static char s_value_buff[4];
    snprintf(s_value_buff, sizeof(s_value_buff), "%d", value);
    graphics_draw_text(
      ctx,
      s_value_buff,
      s_font_s,
      GRect(0, y - 8, 18, 16),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentRight,
      NULL
    );
  }

  graphics_context_set_fill_color(ctx, GColorBlack);

  // Draw Y axis notches at 10% intervals
  for (int i = 0; i <= 10; i++) {
    const int notch_y = root_y + GRAPH_SIZE - ((i * GRAPH_SIZE) / 10);
    graphics_draw_line(
      ctx,
      GPoint(GRAPH_MARGIN - 4, notch_y),
      GPoint(GRAPH_MARGIN, notch_y)
    );
  }

  // Draw X axis notches at x_gap intervals
  for (int i = 0; i < count; i++) {
    const int notch_x = GRAPH_MARGIN + (i * x_gap);
    graphics_draw_line(
      ctx,
      GPoint(notch_x, root_y + GRAPH_SIZE),
      GPoint(notch_x, root_y + GRAPH_SIZE + 4)
    );
  }

  // "Oldest" and "Newest" labels on X axis
  graphics_draw_text(
    ctx,
    "Oldest",
    s_font_s,
    GRect(10, root_y + GRAPH_SIZE + 3, 50, 14),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_text(
    ctx,
    "Newest",
    s_font_s,
    GRect(DISPLAY_W - 50, root_y + GRAPH_SIZE + 3, 50, 14),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );

  int last_x = -1, last_y = -1;

  // Draw points from oldest to newest
  for (int i = 0; i < count; i++) {
    // Moving from oldest to newest
    const int idx = count - i - 1;
    const Sample *s = data_get_sample(idx);
    if (!s || !util_is_not_status(s->charge_perc)) continue;

    const int x = GRAPH_MARGIN + (i * x_gap);
    const int y = root_y + GRAPH_SIZE - ((s->charge_perc * GRAPH_SIZE) / 100);

    // Draw this point
    graphics_fill_circle(ctx, GPoint(x, y), 1);

    if (idx + 1 < count) {
      // The next oldest sample is idx + 1
      const Sample *left_s = data_get_sample(idx + 1);

      // Draw the line to this point IF the previous one had a days_remaining
      if (last_x != -1 && left_s && util_is_not_status(left_s->days_remaining)) {
        graphics_draw_line(ctx, GPoint(last_x, last_y), GPoint(x, y));
      }
      last_x = x;
      last_y = y;
      
      if (left_s && util_is_not_status(left_s->rate)) {
        const int time_diff = s->timestamp - left_s->timestamp;
        const int est_drop = (left_s->rate * time_diff) / SECONDS_PER_DAY; 
        const int est_val = left_s->charge_perc - est_drop;
        const int est_y = root_y + GRAPH_SIZE - ((est_val * GRAPH_SIZE) / 100);
        
        // Draw prediction circle
        graphics_draw_circle(ctx, GPoint(x, est_y), 3);
      }
    }

    // Draw days_remaining cross for this sample
    if (util_is_not_status(s->days_remaining)) {
      const int dr_y = root_y + GRAPH_SIZE - ((s->days_remaining * 100) / GRAPH_SIZE);
      graphics_draw_line(
        ctx,
        GPoint(x - 4, dr_y - 4),
        GPoint(x + 4, dr_y + 4)
      );
      graphics_draw_line(
        ctx,
        GPoint(x - 4, dr_y + 4),
        GPoint(x + 4, dr_y - 4)
      );
    }
  }
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_font_s = fonts_get_system_font(FONT_KEY_S);
  s_font_m = fonts_get_system_font(FONT_KEY_M);

  s_header_layer = util_make_text_layer(GRect(0, -3, bounds.size.w, TITLE_HEIGHT), s_font_m);
  text_layer_set_text(s_header_layer, "Log Graph");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_header_layer));

  s_canvas_layer = layer_create(
    GRect(0, 0, bounds.size.w, bounds.size.h)
  );
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_desc_layer = util_make_text_layer(GRect(4, DESC_Y, bounds.size.w - 8, 100), s_font_s);
  static char s_desc_buff[48];
  snprintf(
    s_desc_buff,
    sizeof(s_desc_buff),
    "Accuracy: %d%%\nDR accuracy: %d%%",
    data_calculate_accuracy(),
    data_calculate_days_remaining_accuracy()
  );
  text_layer_set_text(s_desc_layer, s_desc_buff);
  text_layer_set_overflow_mode(s_desc_layer, GTextOverflowModeWordWrap);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
  text_layer_destroy(s_desc_layer);
  layer_destroy(s_canvas_layer);

  window_destroy(window);
  s_window = NULL;
}

void graph_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
