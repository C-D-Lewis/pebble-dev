#include "main_window.h"

#define TEXT_COLOR GColorWhite
#define LABEL_TEXT_COLOR GColorBlack
#define WINDOW_BG_COLOR PBL_IF_COLOR_ELSE(GColorIslamicGreen, GColorLightGray)
#define WIDGET_BG_COLOR PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorWhite)
#define LABEL_BG_COLOR PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)
#define SHIELD_BAR_COLOR PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite)
#define HEALTH_BAR_COLOR PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)

#define CLASS_ICON_SIZE 40
#define STATUS_ICON_SIZE 16
#define X_ROOT scl_x(30)
#define TIME_Y_ROOT scl_y(440)
#define DATE_Y_ROOT scl_y(804)
#define STATUS_Y_ROOT scl_y(10)
#define DATE_TIME_BG_WIDTH scl_x(938)
#define LABEL_BG_HEIGHT scl_y(89)
#define STATUS_BG_HEIGHT scl_y(315)
#define PROGRESS_BAR_WIDTH scl_x(542)
#define SHIELD_BAR_HEIGHT scl_y(60)
#define HEALTH_BAR_HEIGHT scl_y(71)

#define GRECT_STATUS_BG GRect(0, 0, PS_DISP_W, STATUS_BG_HEIGHT)
#define GRECT_TIME_BG GRect(X_ROOT, TIME_Y_ROOT, DATE_TIME_BG_WIDTH, scl_y(238))
#define GRECT_DATE_BG GRect(X_ROOT, DATE_Y_ROOT, DATE_TIME_BG_WIDTH, scl_y(167))

// TODO: Use Clay and AppMessage / pebble_packet to make this configurable
#define STEP_GOAL 10000
// #define TEST

static Window *s_window;
static TextLayer
  *s_date_layer,
  *s_time_layer,
  *s_time_label_layer,
  *s_date_label_layer,
  *s_class_name_layer;
static BitmapLayer
  *s_class_icon_layer,
  *s_shield_icon_layer,
  *s_health_icon_layer,
  *s_bt_icon_layer;
static Layer *s_shapes_layer;

static GBitmap
  *s_class_icon_bitmap,
  *s_shield_icon_bitmap,
  *s_health_icon_bitmap,
  *s_bt_icon_bitmap;
static GPath *s_time_label_bg_path;
static GPath *s_date_label_bg_path;

static char s_class_name[16];
static int s_class_icon_id;
static int s_battery_perc;
#ifdef TEST
static int s_steps = 3247;
#else
static int s_steps = 0;
#endif

static void update_class_data() {
  int index = rand() % 4;

  switch (index) {
    case 0:
      s_class_icon_id = RESOURCE_ID_CLASS_DRILLER;
      snprintf(s_class_name, sizeof(s_class_name), "DRILLER");
      break;
    case 1:
      s_class_icon_id = RESOURCE_ID_CLASS_ENGINEER;
      snprintf(s_class_name, sizeof(s_class_name), "ENGINEER");
      break;
    case 2:
      s_class_icon_id = RESOURCE_ID_CLASS_GUNNER;
      snprintf(s_class_name, sizeof(s_class_name), "GUNNER");
      break;
    case 3:
    default:
      s_class_icon_id = RESOURCE_ID_CLASS_SCOUT;
      snprintf(s_class_name, sizeof(s_class_name), "SCOUT");
      break;
  }
}

static void shapes_update_proc(Layer *layer, GContext *ctx) {
  // Background areas
#if defined(PBL_COLOR)
  graphics_context_set_fill_color(ctx, WIDGET_BG_COLOR);
  graphics_fill_rect(ctx, GRECT_STATUS_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_TIME_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_DATE_BG, 0, GCornersAll);
#else
  graphics_context_set_fill_color(ctx, WIDGET_BG_COLOR);
  graphics_fill_rect(ctx, GRECT_TIME_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_DATE_BG, 0, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRECT_STATUS_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, grect_inset(GRECT_TIME_BG, GEdgeInsets(1)), 0, GCornersAll);
  graphics_fill_rect(ctx, grect_inset(GRECT_DATE_BG, GEdgeInsets(1)), 0, GCornersAll);
#endif

  // Decoration paths
  graphics_context_set_fill_color(ctx, LABEL_BG_COLOR);
  gpath_draw_filled(ctx, s_time_label_bg_path);
  gpath_draw_filled(ctx, s_date_label_bg_path);

  const int status_x = scl_x(431);
  const int shield_y_offset = scl_y(113);
  const int health_y_offset = scl_y(202);

  // Progress bar backgrounds
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorBlack, WIDGET_BG_COLOR));
  graphics_fill_rect(
    ctx,
    GRect(status_x, STATUS_Y_ROOT + shield_y_offset, PROGRESS_BAR_WIDTH, SHIELD_BAR_HEIGHT),
    0,
    GCornersAll
  );
  graphics_fill_rect(
    ctx,
    GRect(status_x, STATUS_Y_ROOT + health_y_offset, PROGRESS_BAR_WIDTH, HEALTH_BAR_HEIGHT),
    0,
    GCornersAll
  );
#ifdef PBL_BW
  // TODO: draw rect instead of fill rect above on BW
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(
    ctx,
    grect_inset(
      GRect(status_x, STATUS_Y_ROOT + shield_y_offset, PROGRESS_BAR_WIDTH, SHIELD_BAR_HEIGHT),
      GEdgeInsets(1)
    ),
    0,
    GCornersAll
  );
  graphics_fill_rect(
    ctx,
    grect_inset(
      GRect(status_x, STATUS_Y_ROOT + health_y_offset, PROGRESS_BAR_WIDTH, HEALTH_BAR_HEIGHT),
      GEdgeInsets(1)
    ),
    0,
    GCornersAll
  );
#endif

  // Progress bar values
  const int shield_width = (s_battery_perc * PROGRESS_BAR_WIDTH) / 100;
  graphics_context_set_fill_color(ctx, SHIELD_BAR_COLOR);
  graphics_fill_rect(
    ctx,
    GRect(status_x, STATUS_Y_ROOT + shield_y_offset, shield_width, SHIELD_BAR_HEIGHT),
    0,
    GCornersAll
  );

  graphics_context_set_fill_color(ctx, HEALTH_BAR_COLOR);
#if defined(PBL_HEALTH)
  const int health_width = (s_steps * PROGRESS_BAR_WIDTH) / STEP_GOAL;
  graphics_fill_rect(
    ctx,
    GRect(status_x, STATUS_Y_ROOT + health_y_offset, health_width, HEALTH_BAR_HEIGHT),
    0,
    GCornersAll
  );
#else
  graphics_fill_rect(
    ctx,
    GRect(status_x, STATUS_Y_ROOT + health_y_offset, PROGRESS_BAR_WIDTH, HEALTH_BAR_HEIGHT),
    0,
    GCornersAll
  );
#endif
}

#if defined(PBL_HEALTH)
static void update_step_count() {
  HealthMetric metric = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    s_steps = (int)health_service_sum_today(metric);
  } else {
    s_steps = 0;
  }

  layer_mark_dirty(s_shapes_layer);
}

static void health_handler(HealthEventType event, void *context) {
  if (event != HealthEventMovementUpdate) return;

  update_step_count();
}
#endif

static void battery_handler(BatteryChargeState state) {
  s_battery_perc = state.charge_percent;

  layer_mark_dirty(s_shapes_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "tick seconds=%d", tick_time->tm_sec);

  // Time
  static char s_time_buffer[16];
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M HXT", tick_time);
#ifdef TEST
  text_layer_set_text(s_time_layer, "00:00 HXT");
#else
  text_layer_set_text(s_time_layer, s_time_buffer);
#endif

  // First three letters of the day name
  static char s_abrv_day_buffer[4];
  strftime(s_abrv_day_buffer, sizeof(s_abrv_day_buffer), "%a", tick_time);

  // Date
  static char s_date_buffer[24];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%d %b %Y", tick_time);

  // Combine the day name and date
  static char s_day_date_buffer[28];
  snprintf(s_day_date_buffer, sizeof(s_day_date_buffer), "%s %s", s_abrv_day_buffer, s_date_buffer);
#ifdef TEST
  text_layer_set_text(s_date_layer, "MON 01 JAN 2025");
#else
  text_layer_set_text(s_date_layer, s_day_date_buffer);
#endif

  update_class_data();

  // Class Icon
  if (s_class_icon_bitmap) gbitmap_destroy(s_class_icon_bitmap);
  s_class_icon_bitmap = gbitmap_create_with_resource(s_class_icon_id);
  bitmap_layer_set_bitmap(s_class_icon_layer, s_class_icon_bitmap);

#if defined(PBL_HEALTH)
  // Update step count even when no steps are taken (i.e: overnight)
  update_step_count();
#endif
}

 static void bt_handler(bool connected) {
  if (!connected) vibes_double_pulse();

  bitmap_layer_set_bitmap(s_bt_icon_layer, !connected ? s_bt_icon_bitmap : NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Need to do this point by point BECAUSE static is needed AND values must be constant
  static GPoint time_points[6];
  static GPoint date_points[6];

  time_points[0] = GPoint(PBL_IF_COLOR_ELSE(X_ROOT, X_ROOT - 1), TIME_Y_ROOT - LABEL_BG_HEIGHT);
  time_points[1] = GPoint(X_ROOT + scl_x(542), TIME_Y_ROOT - LABEL_BG_HEIGHT);
  time_points[2] = GPoint(X_ROOT + scl_x(625), TIME_Y_ROOT - 3);
  time_points[3] = GPoint(X_ROOT + DATE_TIME_BG_WIDTH, TIME_Y_ROOT - 3);
  time_points[4] = GPoint(X_ROOT + DATE_TIME_BG_WIDTH, TIME_Y_ROOT);
  time_points[5] = GPoint(PBL_IF_COLOR_ELSE(X_ROOT, X_ROOT - 1), TIME_Y_ROOT);

  date_points[0] = GPoint(PBL_IF_COLOR_ELSE(X_ROOT, X_ROOT - 1), DATE_Y_ROOT - LABEL_BG_HEIGHT);
  date_points[1] = GPoint(X_ROOT + scl_x(542), DATE_Y_ROOT - LABEL_BG_HEIGHT);
  date_points[2] = GPoint(X_ROOT + scl_x(625), DATE_Y_ROOT - 3);
  date_points[3] = GPoint(X_ROOT + DATE_TIME_BG_WIDTH, DATE_Y_ROOT - 3);
  date_points[4] = GPoint(X_ROOT + DATE_TIME_BG_WIDTH, DATE_Y_ROOT);
  date_points[5] = GPoint(PBL_IF_COLOR_ELSE(X_ROOT, X_ROOT - 1), DATE_Y_ROOT);

  static GPathInfo time_label_path_info;
  time_label_path_info.num_points = 6;
  time_label_path_info.points = time_points;

  static GPathInfo date_label_path_info;
  date_label_path_info.num_points = 6;
  date_label_path_info.points = date_points;

  s_time_label_bg_path = gpath_create(&time_label_path_info);
  s_date_label_bg_path = gpath_create(&date_label_path_info);

  s_shield_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_SHIELD);
  s_health_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_HEALTH);
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_BT);

  s_shapes_layer = layer_create(bounds);
  layer_set_update_proc(s_shapes_layer, shapes_update_proc);
  layer_add_child(window_layer, s_shapes_layer);

  s_time_layer = text_layer_create(GRect(X_ROOT + 4, TIME_Y_ROOT - 6, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_time_layer, TEXT_COLOR);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, scl_get_font(SFI_Time));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_time_label_layer = text_layer_create(
    GRect(
      X_ROOT + scl_x(27),
      TIME_Y_ROOT - LABEL_BG_HEIGHT - scl_y_pp({.o = 20, .e = 15}),
      bounds.size.w,
      bounds.size.h
    )
  );
  text_layer_set_text_color(s_time_label_layer, LABEL_TEXT_COLOR);
  text_layer_set_background_color(s_time_label_layer, GColorClear);
  text_layer_set_font(s_time_label_layer, scl_get_font(SFI_Label));
  text_layer_set_text(s_time_label_layer, "MISSION TIME");
  layer_add_child(window_layer, text_layer_get_layer(s_time_label_layer));

  s_date_layer = text_layer_create(GRect(X_ROOT + 4, DATE_Y_ROOT, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_date_layer, TEXT_COLOR);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, scl_get_font(SFI_Date));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_date_label_layer = text_layer_create(
    GRect(X_ROOT + 4, DATE_Y_ROOT - LABEL_BG_HEIGHT - 3, bounds.size.w, bounds.size.h)
  );
  text_layer_set_text_color(s_date_label_layer, LABEL_TEXT_COLOR);
  text_layer_set_background_color(s_date_label_layer, GColorClear);
  text_layer_set_font(s_date_label_layer, scl_get_font(SFI_Label));
  text_layer_set_text(s_date_label_layer, "MISSION DATE");
  layer_add_child(window_layer, text_layer_get_layer(s_date_label_layer));

  s_class_name_layer = text_layer_create(GRect(scl_x(313), STATUS_Y_ROOT - 3, PS_DISP_W, 100));
  text_layer_set_text_color(s_class_name_layer, TEXT_COLOR);
  text_layer_set_background_color(s_class_name_layer, GColorClear);
  text_layer_set_font(s_class_name_layer, scl_get_font(SFI_Label));
  text_layer_set_text(s_class_name_layer, s_class_name);
  layer_add_child(window_layer, text_layer_get_layer(s_class_name_layer));

  s_class_icon_layer = bitmap_layer_create(
    GRect(scl_x(10), STATUS_Y_ROOT + scl_y(24), CLASS_ICON_SIZE, CLASS_ICON_SIZE)
  );
  bitmap_layer_set_compositing_mode(s_class_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_class_icon_layer));

  s_shield_icon_layer = bitmap_layer_create(
    GRect(scl_x(299), STATUS_Y_ROOT + scl_y(96), STATUS_ICON_SIZE, STATUS_ICON_SIZE)
  );
  bitmap_layer_set_bitmap(s_shield_icon_layer, s_shield_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_shield_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_shield_icon_layer));

  s_health_icon_layer = bitmap_layer_create(
    GRect(scl_x(299), STATUS_Y_ROOT + scl_y(192), STATUS_ICON_SIZE, STATUS_ICON_SIZE)
  );
  bitmap_layer_set_bitmap(s_health_icon_layer, s_health_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_health_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_health_icon_layer));

  s_bt_icon_layer = bitmap_layer_create(
    GRect(scl_x(875), STATUS_Y_ROOT, STATUS_ICON_SIZE, STATUS_ICON_SIZE)
  );
  bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_icon_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_time_label_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_date_label_layer);
  text_layer_destroy(s_class_name_layer);
  bitmap_layer_destroy(s_class_icon_layer);
  bitmap_layer_destroy(s_shield_icon_layer);
  bitmap_layer_destroy(s_health_icon_layer);
  bitmap_layer_destroy(s_bt_icon_layer);
  layer_destroy(s_shapes_layer);

  gbitmap_destroy(s_class_icon_bitmap);
  gbitmap_destroy(s_shield_icon_bitmap);
  gbitmap_destroy(s_health_icon_bitmap);
  gbitmap_destroy(s_bt_icon_bitmap);

  gpath_destroy(s_time_label_bg_path);
  gpath_destroy(s_date_label_bg_path);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, WINDOW_BG_COLOR);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
#if defined(PBL_HEALTH)
  if (!health_service_events_subscribe(health_handler, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
#endif

  bluetooth_connection_service_subscribe(bt_handler);
  bitmap_layer_set_bitmap(
    s_bt_icon_layer,
    bluetooth_connection_service_peek() ? NULL : s_bt_icon_bitmap
  );

  time_t now = time(NULL);
  struct tm *time_now = localtime(&now);
  tick_handler(time_now, MINUTE_UNIT);

  battery_handler(battery_state_service_peek());

#if defined(PBL_HEALTH)
  health_handler(HealthEventMovementUpdate, NULL);
#endif
}
