#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer;

static GColor s_palette[] = {
  GColorVividCerulean,
  GColorElectricBlue,
  GColorMalachite,
  GColorBrightGreen,
  GColorYellow,
  GColorOrange,
  GColorRed,
  GColorVividViolet
};

static GFont s_font_36, s_font_80;

/*********************************** Drawing **********************************/

static void canvas_layer_update_proc(Layer *layer, GContext *ctx) {
  struct tm *tick_time;
  time_t t = time(NULL);
  tick_time = localtime(&t);

  isometric_begin(ctx);
  isometric_set_projection_offset(PROJECTION_OFFSET);

  // Draw grid
  for (int y = 0; y < GRID_SIZE; y++) {
    for (int x = 0; x < GRID_SIZE; x++) {
      GColor color = s_palette[rand() % NUM_COLORS];
      int z_now = rand() % Z_MOD;
      drawing_draw_square(Vec3(x * B_W, y * B_W, z_now), color);
    }
  }

  isometric_finish(ctx);

  // Draw time
  static char s_time_buff[6];
  strftime(s_time_buff, sizeof(s_time_buff), "%H:%M", tick_time);
  graphics_context_set_text_color(ctx, GColorBlack);
  int x = scl_x_pp({.e = 25, .g = 20});
  int y = scl_y_pp({.e = 180, .g = 210});
  graphics_draw_text(
    ctx,
    s_time_buff,
    s_font_80,
    GRect(x - 3, y - 3, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_time_buff,
    s_font_80,
    GRect(x, y, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Date
  static char s_date_buff[20];
  strftime(s_date_buff, sizeof(s_date_buff), "%e %B %Y", tick_time);

  x = scl_x_pp({.e = -5, .g = -5});
  y = scl_y_pp({.e = 550, .g = 530});
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    s_date_buff,
    s_font_36,
    GRect(x - 2, y - 2, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_date_buff,
    s_font_36,
    GRect(x, y, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Use minute as stable seed
  srand(tick_time->tm_hour + tick_time->tm_min);

  layer_mark_dirty(s_canvas_layer);
}

/************************************ Window **********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_font_36 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_36));
  s_font_80 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FORCED_SQUARE_80));

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_layer_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}
