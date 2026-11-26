#include "main_window.h"

#define IMG_S 80

static Window *s_window;
static Layer *s_canvas_layer;

static GBitmap *s_muninn_bitmap;

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();

  // Draw 'moon'
  graphics_context_set_fill_color(ctx, GColorWhite);
#ifdef PBL_PLATFORM_EMERY
  graphics_fill_rect(ctx, GRect(0, 0, PS_DISP_W, 25), 0, GCornerNone);
#endif
  graphics_fill_circle(ctx, GPoint(PS_DISP_W / 2, scl_y_pp({.o = -10, .e = 80})), PS_DISP_W / 2);

  // Draw time
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  static char s_time_buff[8];
  strftime(s_time_buff, sizeof(s_time_buff), "%H:%M", t);
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    s_time_buff,
    scl_get_font(SFI_Large),
    GRect(0, scl_y_pp({.o = 40, .e = 10}), PS_DISP_W, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Draw Muninn
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(
    ctx,
    s_muninn_bitmap,
    scl_center_x(GRect(0, scl_y_pp({.o = 205, .e = 190}), IMG_S, IMG_S))
  );

  // Days remaining
  static char s_days_buff[24];
  if (app_state->days_remaining != STATUS_EMPTY) {
    snprintf(s_days_buff, sizeof(s_days_buff), "%d days left", app_state->days_remaining);
  } else {
    snprintf(s_days_buff, sizeof(s_days_buff), "--");
  }
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_days_buff,
    scl_get_font(SFI_Medium),
    GRect(0, scl_y_pp({.o = 520, .e = 540}), PS_DISP_W, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Divider
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(
    ctx,
    scl_center_x(GRect(0, scl_y_pp({.o = 650, .e = 670}), scl_x(650), 2)),
    0,
    GCornerNone
  );

  // Battery life
  static char s_life_buff[8];
  if (app_state->battery_life != STATUS_EMPTY) {
    snprintf(s_life_buff, sizeof(s_life_buff), "%d", app_state->battery_life);
  } else {
    snprintf(s_life_buff, sizeof(s_life_buff), "--");
  }
  graphics_draw_text(
    ctx,
    s_life_buff,
    scl_get_font(SFI_Medium),
    GRect(scl_x_pp({.o = 260, .e = 160}), scl_y_pp({.o = 690, .e = 730}), 30, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_draw_text(
    ctx,
    "Total Days",
    scl_get_font(SFI_Small),
    GRect(scl_x_pp({.o = 170, .e = 70}), scl_y_pp({.o = 790, .e = 840}), 100, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );

  // Daily rate
  static char s_rate_buff[8];
  const int last_week_rate = app_state->last_week_rate;
  if (last_week_rate != STATUS_EMPTY) {
    snprintf(s_rate_buff, sizeof(s_rate_buff), "%d%%", last_week_rate);
  } else {
    snprintf(s_rate_buff, sizeof(s_rate_buff), "--");
  }
  graphics_draw_text(
    ctx,
    s_rate_buff,
    scl_get_font(SFI_Medium),
    GRect(scl_x_pp({.o = 580, .e = 630}), scl_y_pp({.o = 690, .e = 730}), 70, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_draw_text(
    ctx,
    "Per Day",
    scl_get_font(SFI_Small),
    GRect(scl_x_pp({.o = 590, .e = 640}), scl_y_pp({.o = 790, .e = 840}), 100, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft,
    NULL
  );
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_muninn_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MUNINN);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  gbitmap_destroy(s_muninn_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

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

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, MINUTE_UNIT);
}

void main_window_reload() {
  layer_mark_dirty(s_canvas_layer);
}
