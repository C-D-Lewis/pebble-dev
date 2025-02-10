#include "main_window.h"

typedef enum {
  FontSizeSmall = 0,
  FontSizeMedium,
  FontSizeLarge
} FontSize;

static GFont s_fonts[3];
static Window *s_window;
static TextLayer *s_brackets_layer, *s_date_layer, *s_time_layer, *s_battery_layer;
static Layer *s_dashes_layer, *s_bt_layer;

static void bt_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, data_get_color(AppKeyBackgroundColor));
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0 , GCornerNone);
}

static void dashes_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int line_height = 2;

  // Draw line
  graphics_context_set_fill_color(ctx, data_get_color(AppKeyLineColor));
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, line_height), 0 , GCornerNone);

  // Cut dashes
  const int num_dashes = 9;
  const int dash_width = 18;
  const int dash_gap = 2;
  graphics_context_set_fill_color(ctx, data_get_color(AppKeyBackgroundColor));
  for(int i = 0; i < num_dashes; i++) {
    GRect rect = GRect((5 * dash_gap) + (i * dash_width), 0, dash_gap, line_height);
    graphics_fill_rect(ctx, rect, 0, GCornerNone);
  }
}

static void bt_handler(bool connected) {
  layer_set_hidden(s_bt_layer, connected);

  if(!connected) {
    vibes_double_pulse();
  }
}

static void batt_handler(BatteryChargeState state) {
  static char batt_buffer[8];
  snprintf(batt_buffer, sizeof(batt_buffer), "%d%%", state.charge_percent);
  text_layer_set_text(s_battery_layer, batt_buffer);
}

static char* get_caps_month(int month) {
  switch(month) {
    case 0: return "JAN";
    case 1: return "FEB";
    case 2: return "MAR";
    case 3: return "APR";
    case 4: return "MAY";
    case 5: return "JUN";
    case 6: return "JUL";
    case 7: return "AUG";
    case 8: return "SEP";
    case 9: return "OCT";
    case 10: return "NOV";
    case 11: return "DEC";
    default: return "UNK";
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Date
  static char date_buffer[16];
  snprintf(date_buffer, sizeof(date_buffer), "%s%d %d", 
    get_caps_month(tick_time->tm_mon), tick_time->tm_mday, tick_time->tm_year - 100);
  text_layer_set_text(s_date_layer, date_buffer);

  // Time
  static char time_buffer[16];
  if(data_get_boolean(AppKeySecondTick)) {
    strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M:%S" : "%I:%M:%S", tick_time);
  } else {
    strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M   " : "%I:%M   ", tick_time);
  }
  text_layer_set_text(s_time_layer, time_buffer);
}

static int get_resource_id(FontSize size) {
  switch(size) {
    case FontSizeSmall:
      return RESOURCE_ID_SHARE_20;
    case FontSizeMedium:
      return RESOURCE_ID_SHARE_24;
    case FontSizeLarge:
      return RESOURCE_ID_SHARE_47;
    default: return 0;
  }
}

static GFont get_font(FontSize size) {
    GFont font = s_fonts[size];
    if(!font) {
        font = fonts_load_custom_font(resource_get_handle(get_resource_id(size)));
        s_fonts[size] = font;
    }
    return font;
}

static TextLayer* make_text_layer(GRect frame, FontSize size) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_text_alignment(this, GTextAlignmentCenter);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_font(this, get_font(size));
  return this;
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  int origin = PBL_IF_ROUND_ELSE(59, 49);
  GRect frame = grect_inset(bounds, GEdgeInsets(origin, -10, 0, -10));
  s_brackets_layer = make_text_layer(frame, FontSizeLarge);
  text_layer_set_text(s_brackets_layer, "[    ]");
  layer_add_child(root_layer, text_layer_get_layer(s_brackets_layer));

  origin += 1;
  frame = grect_inset(bounds, GEdgeInsets(origin, 0));
  s_date_layer = make_text_layer(frame, FontSizeMedium);
  layer_add_child(root_layer, text_layer_get_layer(s_date_layer));

  origin += 28;
  frame = grect_inset(bounds, GEdgeInsets(origin, 0, 0, 0));
  s_time_layer = make_text_layer(frame, FontSizeMedium);
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  origin += 52;
  frame = grect_inset(bounds, GEdgeInsets(origin, 0, 0, 0));
  s_battery_layer = make_text_layer(frame, FontSizeSmall);
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  origin -= PBL_IF_ROUND_ELSE(12, 10);
  s_dashes_layer = layer_create(grect_inset(bounds, GEdgeInsets(origin, 0, 0, 0)));
  layer_set_update_proc(s_dashes_layer, dashes_update_proc);
  layer_add_child(root_layer, s_dashes_layer);

  const int gap = 5;
  const int y_margin = (bounds.size.h - gap) / 2;
  s_bt_layer = layer_create(grect_inset(bounds, GEdgeInsets(y_margin, 0)));
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(root_layer, s_bt_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_brackets_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);

  layer_destroy(s_dashes_layer);

  for (size_t i = 0; i < ARRAY_LENGTH(s_fonts); i++) {
    fonts_unload_custom_font(s_fonts[i]);
  }

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
  }
  window_stack_push(s_window, true);

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, SECOND_UNIT);

  battery_state_service_subscribe(batt_handler);
  batt_handler(battery_state_service_peek());

  main_window_reload();
}

void main_window_reload() {
  layer_set_hidden(text_layer_get_layer(s_battery_layer), !data_get_boolean(AppKeyBatteryMeter));

  layer_set_hidden(s_bt_layer, !data_get_boolean(AppKeyBluetoothAlert));
  connection_service_unsubscribe();
  if(data_get_boolean(AppKeyBluetoothAlert)) {
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bt_handler
    });
    bt_handler(connection_service_peek_pebble_app_connection());
  }

  layer_set_hidden(s_dashes_layer, !data_get_boolean(AppKeyDashedLine));

  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(data_get_boolean(AppKeySecondTick) ? SECOND_UNIT : MINUTE_UNIT, tick_handler);

  window_set_background_color(s_window, data_get_color(AppKeyBackgroundColor));
  text_layer_set_text_color(s_brackets_layer, data_get_color(AppKeyBracketColor));
  text_layer_set_text_color(s_date_layer, data_get_color(AppKeyDateColor));
  text_layer_set_text_color(s_time_layer, data_get_color(AppKeyTimeColor));
  text_layer_set_text_color(s_battery_layer, data_get_color(AppKeyComplicationColor));
}
