#include "log_window.h"

// TODO: Use pebble-scalable when finishing this off

#if defined(PBL_PLATFORM_EMERY)
  #define TITLE_HEIGHT 28
  #define GRAPH_MARGIN 28
  #define DESC_Y 20
  #define GRAPH_SIZE (DISPLAY_W - (2 * GRAPH_MARGIN))
  #define FONT_KEY_M FONT_KEY_GOTHIC_18
  #define FONT_KEY_S FONT_KEY_GOTHIC_14
  #define ZERO_OFFSET 18
#else
  #define TITLE_HEIGHT 22
  #define GRAPH_MARGIN 24
  #define DESC_Y 14
  #define GRAPH_SIZE (DISPLAY_W - (2 * GRAPH_MARGIN))
  #define FONT_KEY_M FONT_KEY_GOTHIC_18
  #define FONT_KEY_S FONT_KEY_GOTHIC_14
  #define ZERO_OFFSET 17
#endif

static Window *s_window;
static TextLayer *s_header_layer, *s_desc_layer;
static Layer *s_canvas_layer;

static GFont s_font_s, s_font_m;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  const int root_y = TITLE_HEIGHT + GRAPH_MARGIN + 5;
  const int x_width = GRAPH_SIZE + (GRAPH_MARGIN / 2);

  // Draw Y and X axes
  graphics_draw_line(
    ctx,
    GPoint(GRAPH_MARGIN, root_y),
    GPoint(GRAPH_MARGIN, root_y + GRAPH_SIZE)
  );
  graphics_draw_line(
    ctx,
    GPoint(GRAPH_MARGIN, root_y + GRAPH_SIZE),
    GPoint(GRAPH_MARGIN + x_width, root_y + GRAPH_SIZE)
  );

  // Draw Y axis labels (0 and 100)
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    "100",
    s_font_s,
    GRect(0, root_y - 8, 18, 32),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );
  graphics_draw_text(
    ctx,
    "0",
    s_font_s,
    GRect(0, GRAPH_SIZE - GRAPH_MARGIN + root_y + ZERO_OFFSET, 18, 32),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );

  // Draw data points
  graphics_context_set_fill_color(ctx, GColorBlack);
  const int count = NUM_SAMPLES;
  const int x_gap = (GRAPH_SIZE + GRAPH_MARGIN) / count;

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

  // Draw points - oldest is on the left
  for (int i = 0; i < count; i++) {
    const Sample *s = data_get_sample(i);
    const int value = s->charge_perc;
    if (!util_is_valid(value)) continue;

    const int x = GRAPH_MARGIN + (i * x_gap);
    const int y = root_y + GRAPH_SIZE - ((value * GRAPH_SIZE) / 100);
    
    graphics_fill_circle(ctx, GPoint(x, y), 2);

    static char s_value_buff[4];
    snprintf(s_value_buff, sizeof(s_value_buff), "%d", value);
    graphics_draw_text(
      ctx,
      s_value_buff,
      s_font_s,
      GRect(x - 12, y - 17, 24, 14),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentCenter,
      NULL
    );

    // Time labels on X axis
    if (i == 0 || i == count - 1) {
      const Sample *label_sample = data_get_sample(count -1 - i);
      const time_t timestamp = label_sample->timestamp;
      struct tm *tm_point = localtime(&timestamp);
      static char s_time_buff[16];
      strftime(s_time_buff, sizeof(s_time_buff), "%H:%M", tm_point);
      graphics_draw_text(
        ctx,
        s_time_buff,
        s_font_s,
        GRect(x - 16, root_y + GRAPH_SIZE + 2, 32, 20),
        GTextOverflowModeTrailingEllipsis,
        GTextAlignmentCenter,
        NULL
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
  text_layer_set_text(s_desc_layer, "Samples are shown oldest to newest.");
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
