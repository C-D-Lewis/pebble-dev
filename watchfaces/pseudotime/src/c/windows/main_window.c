#include "main_window.h"

#define WIDTH PBL_IF_ROUND_ELSE(180, 144)
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)
#define MARGIN PBL_IF_ROUND_ELSE(40, 20)
#define BAR_WIDTH (WIDTH - (MARGIN * 2))
#define BAR_HEIGHT 8
#define BAR_SPACING 12
#define BARS_TOTAL_HEIGHT ((BAR_HEIGHT * 3) + (BAR_SPACING * 2))
#define BARS_Y_OFFSET ((HEIGHT - BARS_TOTAL_HEIGHT) / 2)

// TODO: Dithered grey for monochrome displays
#define BAR_COLOR_HOUR PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)
#define BAR_COLOR_MINUTE PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite)
#define BAR_COLOR_SECOND PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorWhite)
#define BAR_COLOR_BACKGROUND PBL_IF_COLOR_ELSE(GColorDarkGray, GColorBlack)
#define BAR_COLOR_LINE PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite)

static Window *s_window;
static Layer *s_bars_layer;

static int s_hours = 0;
static int s_minutes = 0;
static int s_seconds = 0;

/**
 * Handler when a tick occurs.
 */
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  s_hours = tick_time->tm_hour;
  s_minutes = tick_time->tm_min;
  s_seconds = tick_time->tm_sec;

  layer_mark_dirty(s_bars_layer);
}

/**
 * Draw procedure for the bars layer.
 */
static void bar_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  GRect bar_bounds = GRect(MARGIN, BARS_Y_OFFSET, BAR_WIDTH, BAR_HEIGHT);

  // Draw three bars for hours, minutes, and seconds
  graphics_context_set_fill_color(ctx, BAR_COLOR_BACKGROUND);
  graphics_fill_rect(ctx, bar_bounds, 0, GCornerNone);
  bar_bounds.origin.y += BAR_HEIGHT + BAR_SPACING;
  graphics_fill_rect(ctx, bar_bounds, 0, GCornerNone);
  bar_bounds.origin.y += BAR_HEIGHT + BAR_SPACING;  
  graphics_fill_rect(ctx, bar_bounds, 0, GCornerNone);

  // Draw notch lines on each quarter of each bar
  graphics_context_set_fill_color(ctx, BAR_COLOR_LINE);
  for (int j = 0; j < 3; j++) {
    int notch_x = MARGIN;
    int notch_y = BARS_Y_OFFSET + (j * (BAR_HEIGHT + BAR_SPACING));

    for(int i = 0; i < 5; i++) {
      graphics_fill_rect(ctx, GRect(notch_x, notch_y, 3, BAR_HEIGHT), 0, GCornerNone);
      notch_x += BAR_WIDTH / 4;
    }

    bar_bounds.origin.y += BAR_HEIGHT + BAR_SPACING;
  }

  int hours_progress = s_hours * 100 / 24;
  int minutes_progress = s_minutes * 100 / 60;
  int seconds_progress = s_seconds * 100 / 60;
  int hours_width = (BAR_WIDTH * hours_progress) / 100;
  int minutes_width = (BAR_WIDTH * minutes_progress) / 100;
  int seconds_width = (BAR_WIDTH * seconds_progress) / 100;

  // Draw the progress bars
  graphics_context_set_fill_color(ctx, BAR_COLOR_HOUR);
  graphics_fill_rect(ctx, GRect(MARGIN, BARS_Y_OFFSET, hours_width, BAR_HEIGHT), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, BAR_COLOR_MINUTE);
  graphics_fill_rect(ctx, GRect(MARGIN, BARS_Y_OFFSET + BAR_HEIGHT + BAR_SPACING, minutes_width, BAR_HEIGHT), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, BAR_COLOR_SECOND);
  graphics_fill_rect(ctx, GRect(MARGIN, BARS_Y_OFFSET + (BAR_HEIGHT + BAR_SPACING) * 2, seconds_width, BAR_HEIGHT), 0, GCornerNone);
}

/**
 * Window load event handler.
 */
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bars_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_bars_layer, bar_layer_update_proc);
  
  layer_add_child(window_layer, s_bars_layer);
}

/**
 * Window unload event handler.
 */
static void window_unload(Window *window) {
  layer_destroy(s_bars_layer);

  window_destroy(s_window);
  s_window = NULL;
}

/**
 * Load the main window and push it to the stack.
 */
void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_background_color(s_window, GColorBlack);
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, SECOND_UNIT);
}
