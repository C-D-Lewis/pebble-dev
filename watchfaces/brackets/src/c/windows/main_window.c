#include "main_window.h"

#define WIDTH PBL_IF_ROUND_ELSE(180, 144)
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)

static Window *s_window;
static TextLayer *s_date_layer, *s_time_layer, *s_batt_and_bt_layer, *s_weather_layer;
static BitmapLayer *s_top_brackets_layer, *s_bottom_brackets_layer;
static GBitmap *s_brackets_bitmap;
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
}

static TextLayer* make_text_layer(GRect frame) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_background_color(this, GColorClear);
  text_layer_set_font(this, s_font);
  return this;
}

static bool any_complication_enabled() {
  return data_get_boolean(MESSAGE_KEY_BatteryAndBluetooth); // || weather
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SHARE_24));

  // TODO: based on display HEIGHT
  int initial_y = any_complication_enabled() ? 32 : 55;
  int origin = PBL_IF_ROUND_ELSE(59, initial_y);

  s_brackets_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRACKETS);

  s_top_brackets_layer = bitmap_layer_create(GRect(5, origin, 135, 45));
  bitmap_layer_set_compositing_mode(s_top_brackets_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_top_brackets_layer, s_brackets_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_top_brackets_layer));

  origin -= 7;
  GRect frame = grect_inset(bounds, GEdgeInsets(origin, 0, 0, 20));
  s_date_layer = make_text_layer(frame);
  layer_add_child(root_layer, text_layer_get_layer(s_date_layer));

  origin += 28;
  frame = grect_inset(bounds, GEdgeInsets(origin, 0, 0, 20));
  s_time_layer = make_text_layer(frame);
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  // Always allocate
  s_batt_and_bt_layer = make_text_layer(frame);
  s_weather_layer = make_text_layer(frame);

  // If settings enabled
  if (any_complication_enabled()) {
    origin += 40;

    s_bottom_brackets_layer = bitmap_layer_create(GRect(5, origin, 135, 45));
    bitmap_layer_set_compositing_mode(s_bottom_brackets_layer, GCompOpSet);
    bitmap_layer_set_bitmap(s_bottom_brackets_layer, s_brackets_bitmap);
    layer_add_child(root_layer, bitmap_layer_get_layer(s_bottom_brackets_layer));
  }

  if (data_get_boolean(MESSAGE_KEY_BatteryAndBluetooth)) {
    origin += -7;
    frame = grect_inset(bounds, GEdgeInsets(origin, 0, 0, 20));

    // Battery & BT
    Layer *bbl = text_layer_get_layer(s_batt_and_bt_layer);
    layer_set_frame(bbl, frame);
    layer_add_child(root_layer, bbl);
  }

  if (data_get_boolean(MESSAGE_KEY_WeatherStatus)) {
    origin += 28;
    frame = grect_inset(bounds, GEdgeInsets(origin, 0, 0, 20));

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

  bitmap_layer_destroy(s_top_brackets_layer);
  bitmap_layer_destroy(s_bottom_brackets_layer);

  gbitmap_destroy(s_brackets_bitmap);

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
