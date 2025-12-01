#include "history_window.h"


#if defined(PBL_PLATFORM_EMERY)
  #define TITLE_HEIGHT 28
  #define GRAPH_MARGIN 22
  #define GRAPH_SIZE (DISPLAY_W - (2 * GRAPH_MARGIN))
  #define FONT_KEY_M FONT_KEY_GOTHIC_18
  #define FONT_KEY_S FONT_KEY_GOTHIC_14
#else
  #define TITLE_HEIGHT 28
  #define GRAPH_MARGIN 22
  #define GRAPH_SIZE (DISPLAY_W - (2 * GRAPH_MARGIN))
  #define FONT_KEY_M FONT_KEY_GOTHIC_18
  #define FONT_KEY_S FONT_KEY_GOTHIC_14
#endif

static Window *s_window;
static TextLayer *s_header_layer;
static Layer *s_canvas_layer;

static GFont s_font_s, s_font_m;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  const int root_y = TITLE_HEIGHT + GRAPH_MARGIN;
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

  // Draw Y scale labels (0 and 100)
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
    GRect(0, GRAPH_SIZE - GRAPH_MARGIN + root_y + 12, 18, 32),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentRight,
    NULL
  );

  // Draw data points
  SampleData *data = data_get_sample_data();

  const int count = NUM_STORED_SAMPLES;
  const int x_gap = GRAPH_SIZE / count;

  for (int i = 0; i < count; i++) {
    const int value = data->samples[i].charge_perc;
    const int x = GRAPH_MARGIN + (i * x_gap);
    const int y = root_y + GRAPH_SIZE - ((value * GRAPH_SIZE) / 100);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx, GPoint(x, y), 2);
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
}

static void window_unload(Window *window) {
  text_layer_destroy(s_header_layer);
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
