#include "main_window.h"
#include "cl_util.h"

#define WIDTH PBL_IF_ROUND_ELSE(180, 144)
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)
#define GRECT_SCREEN GRect(0, 0, WIDTH, HEIGHT)

// FIXME: Update coords for chalk
#define STAT_RECT_WIDTH WIDTH - 30
#define WIDGET_RECT_HEIGHT 24
#define SUN_RADIUS (WIDTH / 3) / 2

#define GRECT_TIME_VISIBLE GRect(40, 68, STAT_RECT_WIDTH, WIDGET_RECT_HEIGHT)
#define GRECT_DATE_VISIBLE GRect(27, 123, 90, 30)
#define GRECT_BT_VISIBLE GRect(62, 58, 20, 15)

static Window *s_window;
static BitmapLayer *s_bt_layer;
static GBitmap *s_power_bitmap, *s_bt_on_bitmap, *s_bt_off_bitmap;
static Layer *s_background_layer, *s_phase_layer, *s_stat_bg_layer, *s_battery_layer, *s_date_bg_layer;
static TextLayer *s_time_layer, *s_date_layer;

static AppTimer *s_tap_timer;
static bool tapped = true;

/**
 * Create new rect to relocate a layer by a specific amount.
 */
static GRect move_rect(Layer *l, int x, int y) {
  GRect bounds = layer_get_bounds(l);

  return x > 0
    ? GRect(bounds.origin.x + x, bounds.origin.y, bounds.size.w, bounds.size.h)
    : GRect(bounds.origin.x, bounds.origin.y + y, bounds.size.w, bounds.size.h);
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw sun instead of bitmap
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(WIDTH / 2, HEIGHT / 2), SUN_RADIUS);
}

static void phase_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  int hours = 1;//t->tm_hour;

  if (hours > 0) {
    graphics_context_set_fill_color(ctx, GColorBlack);

    int sun_margin = 2 * SUN_RADIUS;
    int hour_inc = SUN_RADIUS / (clock_is_24h_style() ? 24 : 12);
    int x = sun_margin + (hours * hour_inc);

    // Old algo - ancient magic
    // int x = 19;
    // x += ((hours + 1) * 4);
    // x += minutes / 15;

    graphics_fill_circle(ctx, GPoint(x, HEIGHT / 2), SUN_RADIUS - 4);
  }
}

static void stat_bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  GRect bounds = layer_get_bounds(layer);

  cl_fill_chamfer_rect(ctx, 4, bounds.size.w, bounds.size.h);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(20, 2, 24, 10), 1, GCornersAll);

  BatteryChargeState state = battery_state_service_peek();
  uint8_t perc = state.charge_percent;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(22, 4, 2 * (perc / 10), 6), 1, GCornersAll);

  graphics_draw_bitmap_in_rect(ctx, s_power_bitmap, GRect(8, 0, 8, 14));
}

static void initial_anim_stopped_handler(Animation *animation, bool finished, void *context) {
  tapped = false;
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  // Sun
  s_background_layer = layer_create(GRECT_SCREEN);
  layer_set_update_proc(s_background_layer, (LayerUpdateProc)bg_update_proc);
  layer_add_child(root_layer, s_background_layer);

  // Phase
  s_phase_layer = layer_create(GRECT_SCREEN);
  layer_set_update_proc(s_phase_layer, (LayerUpdateProc)phase_update_proc);
  layer_add_child(root_layer, s_phase_layer);

  // Power bitmap
  // s_power_bitmap = gbitmap_create_with_resource(RESOURCE_ID_POWER);

  // Stat display background
  // s_stat_bg_layer = layer_create(GRECT_SCREEN);
  // s_time_layer = cl_text_layer_create(GRECT_SCREEN, GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  // layer_set_update_proc(s_stat_bg_layer, (LayerUpdateProc)stat_bg_update_proc);
  // layer_add_child(root_layer, s_stat_bg_layer);
  // layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  // Bluetooth
  // s_bt_on_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ON);
  // s_bt_off_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_OFF);
  // s_bt_layer = bitmap_layer_create(GRECT_BT_VISIBLE);
  // layer_add_child(root_layer, bitmap_layer_get_layer(s_bt_layer));

  // Battery
  // s_battery_layer = layer_create(GRECT_BATTERY_VISIBLE);
  // layer_set_update_proc(s_battery_layer, (LayerUpdateProc)battery_update_proc);
  // layer_add_child(root_layer, s_battery_layer);

  // Date
  // s_date_bg_layer = layer_create(GRECT_DATE_BG_VISIBLE);
  // s_date_layer = cl_text_layer_create(GRECT_DATE_BG_VISIBLE, GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  // layer_set_update_proc(s_date_bg_layer, (LayerUpdateProc)stat_bg_update_proc);
  // layer_add_child(root_layer, s_date_bg_layer);
  // layer_add_child(root_layer, text_layer_get_layer(s_date_layer));

  // Animate in
  // cl_animate_layer(s_phase_layer, GRECT_PHASE_HIDDEN, GRECT_PHASE_SHOWING, 1000, 200, initial_anim_stopped_handler);
}

static void window_unload(Window *window) {
  layer_destroy(s_background_layer);
  layer_destroy(s_phase_layer);

  // layer_destroy(s_stat_bg_layer);
  // text_layer_destroy(s_time_layer);

  // gbitmap_destroy(s_power_bitmap);
  // layer_destroy(s_battery_layer);

  // layer_destroy(s_date_bg_layer);
  // text_layer_destroy(s_date_layer);

  // bitmap_layer_destroy(s_bt_layer);
  // gbitmap_destroy(s_bt_on_bitmap);
  // gbitmap_destroy(s_bt_off_bitmap);

  window_destroy(window);
  s_window = NULL;
}

static void tap_timer_callback(void *data) {
  // // Show phase
  // cl_animate_layer(s_phase_layer, layer_get_bounds(s_phase_layer), GRECT_PHASE_VISIBLE, 1000, 0, NULL);

  // // Hide time
  // cl_animate_layer(s_stat_bg_layer, GRECT_STAT_BG_VISIBLE, GRECT_STAT_BG_HIDDEN, 500, 0, NULL);
  // cl_animate_layer(text_layer_get_layer(s_time_layer), GRECT_TIME_VISIBLE, GRECT_TIME_HIDDEN, 500, 0, NULL);

  // // Hide battery
  // cl_animate_layer(s_battery_layer, GRECT_BATTERY_SHOWING, GRECT_BATTERY_HIDDEN, 500, 0, NULL);

  // // Hide date
  // cl_animate_layer(s_date_bg_layer, GRECT_DATE_BG_SHOWING, GRECT_DATE_BG_HIDDEN, 500, 0, NULL);
  // cl_animate_layer(text_layer_get_layer(s_date_layer), GRECT_DATE_SHOWING, GRECT_DATE_BG_HIDDEN, 500, 0, NULL);

  // cl_animate_layer(bitmap_layer_get_layer(s_bt_layer), GRECT_BT_SHOWING, GRECT_BT_HIDDEN, 500, 0, NULL);

  tapped = false;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  if (tapped == true) return;

  // Get bluetooth state
  // bool connected = bluetooth_connection_service_peek();
  // if (connected == true) {
  //   bitmap_layer_set_bitmap(s_bt_layer, s_bt_on_bitmap);
  // } else {
  //   bitmap_layer_set_bitmap(s_bt_layer, s_bt_off_bitmap);
  // }

  // // Hide phase
  // cl_animate_layer(s_phase_layer, layer_get_frame(s_phase_layer), GRect(144, 59, 144, 51), 500, 100, NULL);

  // // Show time
  // cl_animate_layer(s_stat_bg_layer, GRECT_STAT_BG_HIDDEN, GRECT_STAT_BG_SHOWING, 500, 100, NULL);
  // cl_animate_layer(text_layer_get_layer(s_time_layer), GRECT_TIME_HIDDEN, GRECT_TIME_SHOWING, 500, 100, NULL);

  // // Show battery
  // cl_animate_layer(s_battery_layer, GRECT_BATTERY_HIDDEN, GRECT_BATTERY_SHOWING, 500, 100, NULL);

  // // Show date
  // cl_animate_layer(s_date_bg_layer, GRECT_DATE_BG_HIDDEN, GRECT_DATE_BG_SHOWING, 500, 100, NULL);
  // cl_animate_layer(text_layer_get_layer(s_date_layer), GRECT_DATE_BG_HIDDEN, GRECT_DATE_SHOWING, 500, 100, NULL);

  // // Show Bluetooth
  // cl_animate_layer(bitmap_layer_get_layer(s_bt_layer), GRECT_BT_HIDDEN, GRECT_BT_SHOWING, 500, 100, NULL);

  // // Prep disappear
  // s_tap_timer = app_timer_register(5000, (AppTimerCallback)tap_timer_callback, NULL);

  tapped = true;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // static char buff[8];
  // strftime(buff, sizeof(buff), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  // text_layer_set_text(s_time_layer, buff);

  // static char date_buff[16];
  // strftime(date_buff, sizeof(date_buff), "%a %d %b", tick_time);
  // text_layer_set_text(s_date_layer, date_buff);
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });

    accel_tap_service_subscribe(&accel_tap_handler);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }

  window_stack_push(s_window, true);
}
