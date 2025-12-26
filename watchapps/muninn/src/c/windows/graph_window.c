#include "log_window.h"

// TODO: Use pebble-scalable when finishing this off

#if defined(PBL_PLATFORM_EMERY)
  #define TITLE_HEIGHT 28
  #define GRAPH_MARGIN 28
  #define DESC_Y 20
  #define ZERO_OFFSET 18
#else
  #define TITLE_HEIGHT 22
  #define GRAPH_MARGIN 24
  #define DESC_Y 14
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

  // Draw points - oldest is on the left
  int last_x = -1, last_y = -1;
  for (int i = 0; i < count; i++) {
    const Sample *s = data_get_sample(count - i - 1);
    const int value = s->charge_perc;
    if (!util_is_not_status(value)) continue;

    const int x = GRAPH_MARGIN + (i * x_gap);
    const int y = root_y + GRAPH_SIZE - ((value * GRAPH_SIZE) / 100);
    
    // Draw line from previous point
    if (last_x != -1) {
      graphics_draw_line(ctx, GPoint(last_x, last_y), GPoint(x, y));
    }
    last_x = x;
    last_y = y;
    
    graphics_fill_circle(ctx, GPoint(x, y), 1);

    // Draw estimated value given rate at next oldest sample
    if (i != count - 1) {
      const Sample *prev_s = data_get_sample(count - i);
      if (!util_is_not_status(prev_s->rate)) continue;

      const int time_diff = s->timestamp - prev_s->timestamp;
      const int estimated_drop = (prev_s->rate * time_diff) / SECONDS_PER_DAY;
      const int estimated_value = prev_s->charge_perc - estimated_drop;

      const int est_y = root_y + GRAPH_SIZE - ((estimated_value * GRAPH_SIZE) / 100);
      graphics_draw_circle(ctx, GPoint(x, est_y), 2);
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
  static char s_desc_buff[32];
  snprintf(s_desc_buff, sizeof(s_desc_buff), "Accuracy: %d%%", data_calculate_accuracy());
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
