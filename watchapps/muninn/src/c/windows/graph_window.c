#include "log_window.h"

#if defined(PBL_PLATFORM_EMERY)
#else
#endif

#define TITLE_HEIGHT scalable_y(150)
#define GRAPH_MARGIN scalable_x(194)
#define GRAPH_SIZE scalable_x(750)

static Window *s_window;
static TextLayer *s_header_layer, *s_desc_layer;
static Layer *s_canvas_layer;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  const int count = data_get_log_length();
  const int x_gap = GRAPH_SIZE / count;
  const int root_y = scalable_y(160);
  const int last_point_x = GRAPH_MARGIN + ((count - 1) * x_gap);

  // Draw Y and X axes
  graphics_draw_line(
    ctx,
    GPoint(GRAPH_MARGIN, root_y),
    GPoint(GRAPH_MARGIN, root_y + GRAPH_SIZE)
  );
  graphics_draw_line(
    ctx,
    GPoint(GRAPH_MARGIN, root_y + GRAPH_SIZE),
    GPoint(last_point_x, root_y + GRAPH_SIZE)
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
      scalable_get_font(SFI_Small),
      GRect(0, y - scalable_y(70), scalable_x(150), 32),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentRight,
      NULL
    );
  }

  graphics_context_set_fill_color(ctx, GColorBlack);

  // Draw Y axis notches at 20% intervals
  for (int i = 0; i <= 5; i++) {
    const int notch_y = root_y + GRAPH_SIZE - ((i * 20 * GRAPH_SIZE) / 100);
    graphics_draw_line(
      ctx,
      GPoint(GRAPH_MARGIN - 4, notch_y),
      GPoint(GRAPH_MARGIN, notch_y)
    );
  }

  // Draw X axis notches at x_gap intervals, every 4, including last
  for (int i = 0; i < count; i++) {
    if (i % 2 == 0 || i == count - 1) {
      const int notch_x = GRAPH_MARGIN + (i * x_gap);
      graphics_draw_line(
        ctx,
        GPoint(notch_x, root_y + GRAPH_SIZE),
        GPoint(notch_x, root_y + GRAPH_SIZE + 4)
      );
    }
  }

  // Draw 'DD MMM' label below the right-most
  Sample *last_s = data_get_sample(0);
  if (last_s && util_is_not_status(last_s->timestamp)) {
    time_t ts = last_s->timestamp;
    
    static char s_date_buff[8];
    struct tm *time_info = localtime(&ts);
    strftime(s_date_buff, sizeof(s_date_buff), "%d %b", time_info);

    graphics_draw_text(
      ctx,
      s_date_buff,
      scalable_get_font(SFI_Small),
      GRect(last_point_x - scalable_x(120), scalable_y(800), 300, 300),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft,
      NULL
    );
  }

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

  s_header_layer = util_make_text_layer(
    GRect(0, scalable_y_pp(-50, -30), bounds.size.w, TITLE_HEIGHT + 50),
    scalable_get_font(SFI_Medium)
  );
  text_layer_set_text(s_header_layer, "Log Graph");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_header_layer));

  s_canvas_layer = layer_create(
    GRect(0, 0, bounds.size.w, bounds.size.h)
  );
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_desc_layer = util_make_text_layer(
    GRect(2, scalable_y(880), bounds.size.w - 4, 100),
    scalable_get_font(SFI_Small)
  );
  static char s_desc_buff[48];
  snprintf(
    s_desc_buff,
    sizeof(s_desc_buff),
    "Acc R: %d%% DR: %dd",
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
