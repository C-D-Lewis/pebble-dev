#include "main_window.h"
#include "cl_util.h"

#define WIDTH PBL_IF_ROUND_ELSE(180, 144)
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)

// FIXME: Update coords for chalk
#define GRECT_PHASE_HIDDEN GRect(-WIDTH, 0, WIDTH, HEIGHT)
#define GRECT_PHASE_SHOWING GRect(0, 0, WIDTH, HEIGHT)
#define GRECT_TIME_BG_SHOWING GRect(40, 53, 63, 60)
#define GRECT_TIME_BG_HIDDEN GRect(40, HEIGHT, 63, 60)
#define GRECT_TIME_SHOWING GRect(40, 68, 63, 24)
#define GRECT_TIME_HIDDEN GRect(40, 163, 63, 24)
#define GRECT_BATTERY_SHOWING GRect(45, 95, 50, 20)
#define GRECT_BATTERY_HIDDEN GRect(50, HEIGHT, 55, 20)
#define GRECT_DATE_BG_SHOWING GRect(27, 128, 90, 24)
#define GRECT_DATE_BG_HIDDEN GRect(27, HEIGHT, 90, 24)
#define GRECT_DATE_SHOWING GRect(27, 123, 90, 30)
#define GRECT_BT_SHOWING GRect(62, 58, 20, 15)
#define GRECT_BT_HIDDEN GRect(62, HEIGHT, 20, 15)

static Window *s_window;
static BitmapLayer *s_bt_layer;
static GBitmap *s_power_bitmap, *s_bt_on_bitmap, *s_bt_off_bitmap;
static Layer *s_background_layer, *s_phase_layer, *s_time_bg_layer, *s_battery_layer, *s_date_bg_layer;
static TextLayer *s_time_layer, *s_date_layer;

static AppTimer *s_stat_timer;
static bool tapped = true;

static void phase_update_proc(Layer *layer, GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  int hours = t->tm_hour;
  int minutes = t->tm_min;
  // int hours = 12;
  // int minutes = 0;

  if (hours > 0) {
    graphics_context_set_fill_color(ctx, GColorBlack);

    int x = 19;
    x += ((hours + 1) * 4);
    x += minutes / 15;

    graphics_fill_circle(ctx, GPoint(x, HEIGHT / 2), 28);
  }
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw sun instead of bitmap
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(WIDTH / 2, HEIGHT / 2), 32);
}

static void time_bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  GRect bounds = layer_get_bounds(layer);

  // 3 layer filled round rect
  cl_fill_chamfer_rect(ctx, 4, bounds.size.w, bounds.size.h);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(20, 2, 24, 10), 1, GCornersAll);

  graphics_context_set_fill_color(ctx, GColorWhite);
  BatteryChargeState state = battery_state_service_peek();
  uint8_t perc = state.charge_percent;

  graphics_fill_rect(ctx, GRect(22, 4, 2 * (perc / 10), 6), 1, GCornersAll);

  graphics_draw_bitmap_in_rect(ctx, s_power_bitmap, GRect(8, 0, 8, 14));
}

static void initial_anim_stopped_handler(Animation *animation, bool finished, void *context) {
  tapped = false;
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  // Power bitmap
  s_power_bitmap = gbitmap_create_with_resource(RESOURCE_ID_POWER);

  // Time display under background
  s_time_bg_layer = layer_create(GRECT_TIME_BG_HIDDEN);
  s_time_layer = cl_text_layer_create(GRECT_TIME_BG_HIDDEN, GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  layer_set_update_proc(s_time_bg_layer, (LayerUpdateProc)time_bg_update_proc);
  layer_add_child(root_layer, s_time_bg_layer);
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  // Bluetooth
  s_bt_on_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ON);
  s_bt_off_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_OFF);
  s_bt_layer = bitmap_layer_create(GRECT_BT_HIDDEN);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_bt_layer));

  // Battery
  s_battery_layer = layer_create(GRECT_BATTERY_HIDDEN);
  layer_set_update_proc(s_battery_layer, (LayerUpdateProc)battery_update_proc);
  layer_add_child(root_layer, s_battery_layer);

  // Sun
  s_background_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(s_background_layer, (LayerUpdateProc)bg_update_proc);
  layer_add_child(root_layer, s_background_layer);

  // Phase
  s_phase_layer = layer_create(GRECT_PHASE_SHOWING);
  layer_set_update_proc(s_phase_layer, (LayerUpdateProc)phase_update_proc);
  layer_add_child(root_layer, s_phase_layer);

  // Date
  s_date_bg_layer = layer_create(GRECT_DATE_BG_HIDDEN);
  s_date_layer = cl_text_layer_create(GRECT_DATE_BG_HIDDEN, GColorBlack, GColorClear, false, 0, FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter);
  layer_set_update_proc(s_date_bg_layer, (LayerUpdateProc)time_bg_update_proc);
  layer_add_child(root_layer, s_date_bg_layer);
  layer_add_child(root_layer, text_layer_get_layer(s_date_layer));

  // Animate in
  cl_animate_layer(s_phase_layer, GRECT_PHASE_HIDDEN, GRECT_PHASE_SHOWING, 1000, 200, initial_anim_stopped_handler);
}

static void window_unload(Window *window) {
  layer_destroy(s_background_layer);
  layer_destroy(s_phase_layer);

  layer_destroy(s_time_bg_layer);
  text_layer_destroy(s_time_layer);

  gbitmap_destroy(s_power_bitmap);
  layer_destroy(s_battery_layer);

  layer_destroy(s_date_bg_layer);
  text_layer_destroy(s_date_layer);

  bitmap_layer_destroy(s_bt_layer);
  gbitmap_destroy(s_bt_on_bitmap);
  gbitmap_destroy(s_bt_off_bitmap);

  window_destroy(window);
  s_window = NULL;
}

static void stat_timer_callback(void *data) {
  // Show phase
  cl_animate_layer(s_phase_layer, GRECT_PHASE_HIDDEN, GRECT_PHASE_SHOWING, 1000, 0, NULL);

  // Hide time
  cl_animate_layer(s_time_bg_layer, GRECT_TIME_BG_SHOWING, GRECT_TIME_BG_HIDDEN, 500, 0, NULL);
  cl_animate_layer(text_layer_get_layer(s_time_layer), GRECT_TIME_SHOWING, GRECT_TIME_HIDDEN, 500, 0, NULL);

  // Hide battery
  cl_animate_layer(s_battery_layer, GRECT_BATTERY_SHOWING, GRECT_BATTERY_HIDDEN, 500, 0, NULL);

  // Hide date
  cl_animate_layer(s_date_bg_layer, GRECT_DATE_BG_SHOWING, GRECT_DATE_BG_HIDDEN, 500, 0, NULL);
  cl_animate_layer(text_layer_get_layer(s_date_layer), GRECT_DATE_SHOWING, GRECT_DATE_BG_HIDDEN, 500, 0, NULL);

  cl_animate_layer(bitmap_layer_get_layer(s_bt_layer), GRECT_BT_SHOWING, GRECT_BT_HIDDEN, 500, 0, NULL);

  tapped = false;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  if (tapped == false) {
    // Get bluetooth state
    bool connected = bluetooth_connection_service_peek();
    if (connected == true) {
      bitmap_layer_set_bitmap(s_bt_layer, s_bt_on_bitmap);
    } else {
      bitmap_layer_set_bitmap(s_bt_layer, s_bt_off_bitmap);
    }

    // Hide phase
    cl_animate_layer(s_phase_layer, layer_get_frame(s_phase_layer), GRect(144, 59, 144, 51), 500, 100, NULL);

    // Show time
    cl_animate_layer(s_time_bg_layer, GRECT_TIME_BG_HIDDEN, GRECT_TIME_BG_SHOWING, 500, 100, NULL);
    cl_animate_layer(text_layer_get_layer(s_time_layer), GRECT_TIME_HIDDEN, GRECT_TIME_SHOWING, 500, 100, NULL);

    // Show battery
    cl_animate_layer(s_battery_layer, GRECT_BATTERY_HIDDEN, GRECT_BATTERY_SHOWING, 500, 100, NULL);

    // Show date
    cl_animate_layer(s_date_bg_layer, GRECT_DATE_BG_HIDDEN, GRECT_DATE_BG_SHOWING, 500, 100, NULL);
    cl_animate_layer(text_layer_get_layer(s_date_layer), GRECT_DATE_BG_HIDDEN, GRECT_DATE_SHOWING, 500, 100, NULL);

    // Animate bluetooth layer
    cl_animate_layer(bitmap_layer_get_layer(s_bt_layer), GRECT_BT_HIDDEN, GRECT_BT_SHOWING, 500, 100, NULL);

    // Prep dissapear
    s_stat_timer = app_timer_register(5000, (AppTimerCallback)stat_timer_callback, NULL);

    tapped = true;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  static char buff[8];
  strftime(buff, 8 * sizeof(char), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, buff);

  static char date_buff[16];
  strftime(date_buff, 16 * sizeof(char), "%a %d %b", tick_time);
  text_layer_set_text(s_date_layer, date_buff);
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
