#include <pebble.h>
#include "lib/InverterLayerCompat.h"

static Window *window;
static GBitmap *background_bitmap;
static BitmapLayer *background_layer;
static TextLayer *hours_layer, *minutes_layer, *day_layer, *voltage_layer;
static InverterLayerCompat *electron_layer;

static bool setup = false;
static int x = 21, y = 78, w = 3, h = 3;

static TextLayer* make_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_load_custom_font(handle));
  text_layer_set_text_alignment(layer, alignment);

  return layer;
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void tick_handler(struct tm *t, TimeUnits unit_changed) {
  int days = t->tm_mday;
  int hours = t->tm_hour;
  int minutes = t->tm_min;
  int seconds = t->tm_sec;

  // Update resistors and output voltage
  if (seconds % 60 == 0 || !setup) {
    // Day of the month is the battery voltage
    static char day_buff[] = "XXV";
    snprintf(day_buff, sizeof("XXV"), "%dV", days);
    text_layer_set_text(day_layer, day_buff);

    // R1 is the hours
    static char hour_buff[] = "XX";
    if (clock_is_24h_style()) {
      strftime(hour_buff, sizeof("XX"), "%H", t);
    } else {
      strftime(hour_buff, sizeof("XX"), "%I", t);
    }
    text_layer_set_text(hours_layer, hour_buff);

    // R2 is the minutes
    static char min_buff[] = "XX";
    strftime(min_buff, sizeof("XX"), "%M", t);
    text_layer_set_text(minutes_layer, min_buff);

    // Output voltage
    // Vout = (Vin * R2) / (R1 + R2)
    int daysMv = days * 1000; // Convert to millivolts
    int voltageMv = (daysMv * minutes) / (hours + minutes);
    int volts = voltageMv / 1000;
    int frac = (voltageMv % 1000) / 100;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "intToVoltageString: %d mV -> %d.%03d V", millivolts, volts, frac);
    static char voltage_buff[16];
    snprintf(voltage_buff, sizeof(voltage_buff), "%d.%01dV", volts, frac);
    text_layer_set_text(voltage_layer, voltage_buff);
  }

  // Update moving electron
  if (seconds == 0 || !setup) {
    // Reset position
    x = 21;
    y = 78;
    setup = true;
  }
  if (seconds < 10) {
    y -= 6;
    if (x != 21) {
      x = 21;
    }
  }
  if (seconds == 10) {
    y = 16;
  }
  if (seconds > 10 && seconds < 20) {
    x += 7;
    if (y != 16) {
      y = 16;
    }
  }
  if (seconds == 20) {
    x = 94;
  }
  if (seconds > 20 && seconds < 40) {
    y += 7;
    if (x != 94) {
      x = 94;
    }
  }
  if (seconds == 40) {
    y = 151;
  }
  if (seconds > 40 && seconds < 51) {
    x -= 7;
    if (y != 151) {
     y = 151;
    }
  }
  if (seconds == 51) {
    x = 21;
  }
  if (seconds > 51 && seconds < 60) {
    y -= 7;
    if (x != 21) {
      x = 21;
    }
  }

  // Draw moving electron IF it has been setup with the correct initial position
  Layer *electron_layer_layer = inverter_layer_compat_get_layer(electron_layer);
  layer_set_frame(electron_layer_layer, GRect(x, y, w, h));
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  // Load resources
  background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);

  // Background
  background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(background_layer, background_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(background_layer));

  // Electron
  electron_layer = inverter_layer_compat_create(GRect(0, 0, 0, 0), GColorWhite, GColorBlack);
  Layer *electron_layer_layer = inverter_layer_compat_get_layer(electron_layer);
  layer_add_child(root_layer, electron_layer_layer);

  // TextLayers
  day_layer = make_text_layer(GRect(40, 76, 30, 30), GColorWhite, GColorClear, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  layer_add_child(root_layer, text_layer_get_layer(day_layer));

  hours_layer = make_text_layer(GRect(60, 42, 30, 30), GColorWhite, GColorClear, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  layer_add_child(root_layer, text_layer_get_layer(hours_layer));

  minutes_layer = make_text_layer(GRect(60, 109, 30, 30), GColorWhite, GColorClear, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  layer_add_child(root_layer, text_layer_get_layer(minutes_layer));

  voltage_layer = make_text_layer(GRect(112, 60, 60, 30), GColorWhite, GColorClear, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  layer_add_child(root_layer, text_layer_get_layer(voltage_layer));
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(background_layer);
  inverter_layer_compat_destroy(electron_layer);
  text_layer_destroy(hours_layer);
  text_layer_destroy(minutes_layer);
  text_layer_destroy(day_layer);
  text_layer_destroy(voltage_layer);

  gbitmap_destroy(background_bitmap);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
