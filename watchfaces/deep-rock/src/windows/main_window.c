#include <pebble.h>

// #define TEST

#define TEXT_COLOR GColorWhite
#define LABEL_TEXT_COLOR GColorBlack
#define WINDOW_BG_COLOR PBL_IF_COLOR_ELSE(GColorIslamicGreen, GColorBlack)
#define ITEM_BG_COLOR PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorWhite)
#define LABEL_BG_COLOR PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)

#define X_MARGIN 5
#define TIME_Y_ROOT 68
#define DATE_Y_ROOT 132
#define DATE_TIME_BG_WIDTH 135
#define LABEL_BG_HEIGHT 15
#define STATUS_BG_HEIGHT 45

#define GRECT_STATUS_BG GRect(0, 0, 144, STATUS_BG_HEIGHT)
#define GRECT_TIME_BG GRect(X_MARGIN, TIME_Y_ROOT, DATE_TIME_BG_WIDTH, 40)
#define GRECT_DATE_BG GRect(X_MARGIN, DATE_Y_ROOT, DATE_TIME_BG_WIDTH, 28)

static Window *s_window;
static TextLayer *s_date_layer, *s_time_layer, *s_time_label_layer, *s_date_label_layer;
static BitmapLayer *s_class_icon_layer;
static Layer *s_item_bg_layer;

static GBitmap *s_class_icon_bitmap;

static GPath *s_time_label_bg_path;
static GPath *s_date_label_bg_path;
static const GPathInfo TIME_LABEL_PATH_INFO = {
  .num_points = 6,
  .points = (GPoint []) {
    {PBL_IF_COLOR_ELSE(X_MARGIN, X_MARGIN - 1), TIME_Y_ROOT - LABEL_BG_HEIGHT},
    {X_MARGIN + 78, TIME_Y_ROOT - LABEL_BG_HEIGHT},
    {X_MARGIN + 90, TIME_Y_ROOT - 3},
    {X_MARGIN + DATE_TIME_BG_WIDTH, TIME_Y_ROOT - 3},
    {X_MARGIN + DATE_TIME_BG_WIDTH, TIME_Y_ROOT},
    {PBL_IF_COLOR_ELSE(X_MARGIN, X_MARGIN - 1), TIME_Y_ROOT}
  }
};
static const GPathInfo DATE_LABEL_PATH_INFO = {
  .num_points = 6,
  .points = (GPoint []) {
    {PBL_IF_COLOR_ELSE(X_MARGIN, X_MARGIN - 1), DATE_Y_ROOT - LABEL_BG_HEIGHT},
    {X_MARGIN + 78, DATE_Y_ROOT - LABEL_BG_HEIGHT},
    {X_MARGIN + 90, DATE_Y_ROOT - 3},
    {X_MARGIN + DATE_TIME_BG_WIDTH, DATE_Y_ROOT - 3},
    {X_MARGIN + DATE_TIME_BG_WIDTH, DATE_Y_ROOT},
    {PBL_IF_COLOR_ELSE(X_MARGIN, X_MARGIN - 1), DATE_Y_ROOT}
  }
};

static void create_paths() {
  s_time_label_bg_path = gpath_create(&TIME_LABEL_PATH_INFO);
  s_date_label_bg_path = gpath_create(&DATE_LABEL_PATH_INFO);
}

static void destroy_paths() {
  gpath_destroy(s_time_label_bg_path);
  gpath_destroy(s_date_label_bg_path);
}

static void item_bg_update_proc(Layer *layer, GContext *ctx) {
  // Background areas
#if defined(PBL_COLOR)
  graphics_context_set_fill_color(ctx, ITEM_BG_COLOR);
  graphics_fill_rect(ctx, GRECT_STATUS_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_TIME_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_DATE_BG, 0, GCornersAll);
#else
  graphics_context_set_fill_color(ctx, ITEM_BG_COLOR);
  graphics_fill_rect(ctx, GRECT_STATUS_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_TIME_BG, 0, GCornersAll);
  graphics_fill_rect(ctx, GRECT_DATE_BG, 0, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, grect_inset(GRECT_STATUS_BG, GEdgeInsets(1)), 0, GCornersAll);
  graphics_fill_rect(ctx, grect_inset(GRECT_TIME_BG, GEdgeInsets(1)), 0, GCornersAll);
  graphics_fill_rect(ctx, grect_inset(GRECT_DATE_BG, GEdgeInsets(1)), 0, GCornersAll);
#endif

  // Decoration paths
  graphics_context_set_fill_color(ctx, LABEL_BG_COLOR);
  gpath_draw_filled(ctx, s_time_label_bg_path);
  gpath_draw_filled(ctx, s_date_label_bg_path);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Time
  static char s_time_buffer[16];
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M HXT", tick_time);
#ifdef TEST
  text_layer_set_text(s_time_layer, "23:00 HXT");
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
  text_layer_set_text(s_date_layer, "SUN 29 MAY 2022");
#else
  text_layer_set_text(s_date_layer, s_day_date_buffer);
#endif

  // Steps

  // Calories
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_item_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_item_bg_layer, item_bg_update_proc);
  layer_add_child(window_layer, s_item_bg_layer);

  GFont ostrich_regular_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_REGULAR_16));
  GFont ostrich_heavy_22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_HEAVY_22));
  GFont ostrich_heavy_40 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OSTRICH_HEAVY_40));

  s_time_layer = text_layer_create(GRect(X_MARGIN + 4, TIME_Y_ROOT - 6, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_time_layer, TEXT_COLOR);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, ostrich_heavy_40);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_time_label_layer = text_layer_create(GRect(X_MARGIN + 4, TIME_Y_ROOT - LABEL_BG_HEIGHT - 3, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_time_label_layer, LABEL_TEXT_COLOR);
  text_layer_set_background_color(s_time_label_layer, GColorClear);
  text_layer_set_font(s_time_label_layer, ostrich_regular_16);
  text_layer_set_text(s_time_label_layer, "MISSION TIME");
  layer_add_child(window_layer, text_layer_get_layer(s_time_label_layer));

  s_date_layer = text_layer_create(GRect(X_MARGIN + 4, DATE_Y_ROOT, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_date_layer, TEXT_COLOR);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, ostrich_heavy_22);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_date_label_layer = text_layer_create(GRect(X_MARGIN + 4, DATE_Y_ROOT - LABEL_BG_HEIGHT - 3, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_date_label_layer, LABEL_TEXT_COLOR);
  text_layer_set_background_color(s_date_label_layer, GColorClear);
  text_layer_set_font(s_date_label_layer, ostrich_regular_16);
  text_layer_set_text(s_date_label_layer, "MISSION DATE");
  layer_add_child(window_layer, text_layer_get_layer(s_date_label_layer));

  s_class_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLASS_ICON_SCOUT);

  s_class_icon_layer = bitmap_layer_create(GRect(2, 2, 40, 40));
  bitmap_layer_set_bitmap(s_class_icon_layer, s_class_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_class_icon_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_class_icon_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_time_label_layer);
  text_layer_destroy(s_date_layer);
  bitmap_layer_destroy(s_class_icon_layer);
  layer_destroy(s_item_bg_layer);

  gbitmap_destroy(s_class_icon_bitmap);
  destroy_paths();

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

    create_paths();
  }

  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_t now = time(NULL);
  struct tm *time_now = localtime(&now);
  tick_handler(time_now, MINUTE_UNIT);
}
