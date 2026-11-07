#if defined(PBL_ROUND)
#include "line_window.h"

#define RING_MARGIN scl_x(111)
#define RING_MAX_ANGLE 320
#define RADIUS scl_x(60)
#define MARGIN scl_x(50)
#define STROKE_WIDTH scl_x(15)

static Window *s_window;
static Layer *s_ring_layer;

static GTextAttributes *s_attributes;
static GBitmap *s_spanner_bitmap;
static int s_selected_line;

/********************************** Drawing ***********************************/

static void draw_station(GContext *ctx, GPoint center, int index) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, (6 * (RADIUS - 1)) / 7);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, STROKE_WIDTH);
  graphics_draw_circle(ctx, center, RADIUS - 2);
}

/******************************** Click Config ********************************/

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_selected_line == 0) return;

  s_selected_line -= 1;
  layer_mark_dirty(s_ring_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  const int max = data_get_lines_received();
  if (s_selected_line == max) return;

  s_selected_line += 1;
  layer_mark_dirty(s_ring_layer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  int max = data_get_lines_received();
  if (s_selected_line >= max || !data_get_line_has_reason(s_selected_line)) return;

  reason_window_push(s_selected_line);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/*********************************** Window ***********************************/

static void ring_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  const LineData *line_data = data_get_line(s_selected_line);
  const int name_y_margin = scl_y(360);
  const GSize name_size = graphics_text_layout_get_content_size_with_attributes(
    data_get_line_name(line_data->type),
    scl_get_font(SFI_MediumBold),
    grect_inset(bounds, GEdgeInsets(name_y_margin, 0, 0, 0)),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    s_attributes
  );

  const int num_received = data_get_lines_received();

  // Draw ring
  GColor line_color = s_selected_line == num_received
    ? GColorBlue
    : data_get_line_color(line_data->type);
  graphics_context_set_fill_color(ctx, line_color);
  GRect ring_rect = grect_inset(bounds, GEdgeInsets(RING_MARGIN));
  graphics_fill_radial(
    ctx,
    ring_rect,
    GOvalScaleModeFitCircle,
    MARGIN,
    DEG_TO_TRIGANGLE(0),
    DEG_TO_TRIGANGLE(360)
  );
  if (s_selected_line != num_received && data_get_line_color_is_striped(line_data->type)) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_radial(
      ctx,
      grect_inset(ring_rect, GEdgeInsets(scl_x(15))),
      GOvalScaleModeFitCircle,
      scl_x(20),
      DEG_TO_TRIGANGLE(0),
      DEG_TO_TRIGANGLE(360)
    );
  }

  // Stations
  int angle = 0;
  for(int i = 0; i <= num_received; i++) {
    angle = (i * RING_MAX_ANGLE) / num_received;
    GPoint center = gpoint_from_polar(
      grect_inset(ring_rect, GEdgeInsets((MARGIN / 2) - 1)),
      GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(angle)
    );
    draw_station(ctx, center, i);
  }

  // Selection
  angle = (s_selected_line * RING_MAX_ANGLE) / num_received;
  GPoint center = gpoint_from_polar(
    grect_inset(ring_rect, GEdgeInsets((MARGIN / 2) - 1)),
    GOvalScaleModeFitCircle,
    DEG_TO_TRIGANGLE(angle)
  );
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, (MARGIN / 2) + scl_x(5));

  graphics_context_set_text_color(ctx, GColorBlack);

  // Summary of all other statuses
  if (s_selected_line == num_received) {
    graphics_draw_text(
      ctx,
      num_received == 0 ? "Good service on all lines" : "Good service on all other lines",
      scl_get_font(SFI_Medium),
      grect_inset(bounds, GEdgeInsets(name_y_margin, 0, 0, 0)),
      GTextOverflowModeWordWrap,
      GTextAlignmentCenter,
      s_attributes
    );
    return;
  }

  // Line name and status
  graphics_draw_text(
    ctx,
    data_get_line_name(line_data->type),
    scl_get_font(SFI_MediumBold),
    grect_inset(bounds, GEdgeInsets(name_y_margin, 0, 0, 0)),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    s_attributes
  );
  graphics_draw_text(
    ctx,
    line_data->state,
    scl_get_font(SFI_Medium),
    grect_inset(bounds, GEdgeInsets(name_y_margin + name_size.h, 0, 0, 0)),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    s_attributes
  );

  // Reason window hint
  if (data_get_line_has_reason(s_selected_line)) {
    const int reason_inset = scl_x(60);
    // Background
    graphics_context_set_fill_color(ctx, data_get_line_state_color(s_selected_line));
    graphics_fill_rect(
      ctx,
      grect_inset(bounds, GEdgeInsets(0, 0, 0, bounds.size.w - reason_inset)),
      GCornerNone,
      0
    );

    // Arrow
    graphics_draw_text(
      ctx,
      " >",
      scl_get_font(SFI_MediumBold),
      GRect(bounds.size.w - reason_inset, (bounds.size.h / 2) - 20, 30, 30),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft,
      NULL
    );
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_spanner_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPANNER);

  s_ring_layer = layer_create(bounds);
  layer_set_update_proc(s_ring_layer, ring_update_proc);
  layer_add_child(window_layer, s_ring_layer);

  s_attributes = graphics_text_attributes_create();
  graphics_text_attributes_enable_screen_text_flow(s_attributes, RING_MARGIN + MARGIN);
}

static void window_unload(Window *window) {
  layer_destroy(s_ring_layer);
  graphics_text_attributes_destroy(s_attributes);
  gbitmap_destroy(s_spanner_bitmap);

  window_destroy(s_window);
  s_window = NULL;
  window_stack_pop_all(true);  // Don't show splash on exit
}

void line_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  // Begin with the first line
  s_selected_line = 0;
  layer_mark_dirty(s_ring_layer);
}
#endif
