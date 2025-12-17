#include "main_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define DISPLAY_W 200
  #define DISPLAY_H 228
#elif defined(PBL_PLATFORM_CHALK)
  #define DISPLAY_W 180
  #define DISPLAY_H 180
#else
  #define DISPLAY_W 144
  #define DISPLAY_H 168
#endif

static Window *s_window;
static TextLayer *s_date_layer, *s_time_layer, *s_batt_and_bt_layer, *s_weather_layer;
static Layer *s_canvas_layer;
static GFont s_font;

static void update_batt_and_bt() {
  const bool connected = connection_service_peek_pebble_app_connection();
  const BatteryChargeState state = battery_state_service_peek();
  const uint8_t perc = state.charge_percent;

  // Must be a smarter way...
  static char s_buffer[16];
  if (connected) {
    if (perc == 100) {
      snprintf(s_buffer, sizeof(s_buffer), "%d%%  BT", perc);
    } else {
      snprintf(s_buffer, sizeof(s_buffer), "%d%%   BT", perc);
    }
  } else {
    snprintf(s_buffer, sizeof(s_buffer), "%d%%", perc);
  }
  text_layer_set_text(s_batt_and_bt_layer, s_buffer);
}

static void bt_handler(bool connected) {
  update_batt_and_bt();

  if (!connected) vibes_double_pulse();
}

static void batt_handler(BatteryChargeState state) {
  update_batt_and_bt();
}

static char* get_uppercase_month(int month) {
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

static char* zero_pad(int value) {
  static char buffer[8];
  snprintf(buffer, sizeof(buffer), "%02d", value);
  return buffer;
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Date
  static char date_buffer[16];
  snprintf(
    date_buffer,
    sizeof(date_buffer),
    "%s%s %d",
    get_uppercase_month(tick_time->tm_mon),
    zero_pad(tick_time->tm_mday),
    tick_time->tm_year - 100
  );
  text_layer_set_text(s_date_layer, date_buffer);

  // Time
  static char time_buffer[16];
  if (data_get_boolean(MESSAGE_KEY_SecondTick)) {
    strftime(
      time_buffer,
      sizeof(time_buffer),
      clock_is_24h_style() ? "%H:%M:%S" : "%I:%M:%S",
      tick_time
    );
  } else {
    strftime(
      time_buffer,
      sizeof(time_buffer),
      clock_is_24h_style() ? "%H:%M   " : "%I:%M   ",
      tick_time
    );
  }
  text_layer_set_text(s_time_layer, time_buffer);

  // Weather refresh
  if ((tick_time->tm_min == 0 || tick_time->tm_min == 30) && tick_time->tm_sec == 0) {
    if (data_get_boolean(MESSAGE_KEY_WeatherStatus)) {
      comm_request_weather();
    }
  }
}

static TextLayer* make_text_layer(GRect frame) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_font(this, scalable_get_font(SFI_Medium));
  return this;
}

static bool any_complication_enabled() {
  return data_get_boolean(MESSAGE_KEY_BatteryAndBluetooth) || data_get_boolean(MESSAGE_KEY_WeatherStatus);
}

static void draw_brackets(GContext *ctx, GRect bounds, int scl_y) {
  GRect rect = scalable_grect(0, scl_y, 940, 270);
  const int bracket_w = scalable_x(30);
  const int bracket_x_inset = scalable_x(70);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, rect, GCornerNone, 0);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(
    ctx,
    grect_inset(rect, GEdgeInsets(bracket_w)),
    GCornerNone,
    0
  );
  graphics_fill_rect(
    ctx,
    grect_inset(rect, GEdgeInsets(0, bracket_x_inset, 0, bracket_x_inset)),
    GCornerNone,
    0
  );
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  draw_brackets(ctx, bounds, 0);

  if (any_complication_enabled()) {
    draw_brackets(ctx, bounds, 365);
  }
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  const int root_x = PBL_IF_ROUND_ELSE(scalable_x(150), scalable_x(25));
  const int root_y = any_complication_enabled()
    ? scalable_y(190)
    : scalable_y(360);
  int y = PBL_IF_ROUND_ELSE(scalable_y(200), root_y);

  s_canvas_layer = layer_create(GRect(root_x, root_y, DISPLAY_W, DISPLAY_H));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  const int text_x = PBL_IF_ROUND_ELSE(scalable_x(260), scalable_x_pp(140, 113));
  y -= scalable_y_pp(40, 50);
  GRect frame = grect_inset(bounds, GEdgeInsets(y, 0, 0, text_x));
  s_date_layer = make_text_layer(frame);
  layer_add_child(root_layer, text_layer_get_layer(s_date_layer));

  y += scalable_y_pp(165, 162);
  frame = grect_inset(bounds, GEdgeInsets(y, 0, 0, text_x));
  s_time_layer = make_text_layer(frame);
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  // Always allocate
  s_batt_and_bt_layer = make_text_layer(frame);
  s_weather_layer = make_text_layer(frame);

  y += scalable_y_pp(195, 200);

  if (data_get_boolean(MESSAGE_KEY_BatteryAndBluetooth)) {
    frame = grect_inset(bounds, GEdgeInsets(y, 0, 0, text_x));

    // Battery & BT
    Layer *bbl = text_layer_get_layer(s_batt_and_bt_layer);
    layer_set_frame(bbl, frame);
    layer_add_child(root_layer, bbl);
  }

  if (data_get_boolean(MESSAGE_KEY_WeatherStatus)) {
    y += scalable_y_pp(165, 163);
    frame = grect_inset(bounds, GEdgeInsets(y, 0, 0, text_x));

    // Weather
    Layer *wl = text_layer_get_layer(s_weather_layer);
    layer_set_frame(wl, frame);
    layer_add_child(root_layer, wl);
  }
}

static void window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_batt_and_bt_layer);
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_font);

  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
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

  data_set_weather_string("...  ...");
  main_window_reload();
}

void main_window_reload() {
  // Battery and Bluetooth
  if (!data_get_boolean(MESSAGE_KEY_BatteryAndBluetooth)) {
    layer_set_hidden(text_layer_get_layer(s_batt_and_bt_layer), true);

    connection_service_unsubscribe();
    battery_state_service_unsubscribe();
  } else {
    layer_set_hidden(text_layer_get_layer(s_batt_and_bt_layer), false);

    battery_state_service_subscribe(batt_handler);
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bt_handler
    });

    update_batt_and_bt();
  }

  // Weather
  if (!data_get_boolean(MESSAGE_KEY_WeatherStatus)) {
    layer_set_hidden(text_layer_get_layer(s_weather_layer), true);
  } else {
    layer_set_hidden(text_layer_get_layer(s_weather_layer), false);
    text_layer_set_text(s_weather_layer, data_get_weather_string());
  }

  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(data_get_boolean(MESSAGE_KEY_SecondTick) ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

/** Reload only data from JS, not all subscriptions */
void main_window_update_data() {
  text_layer_set_text(s_weather_layer, data_get_weather_string());
}
