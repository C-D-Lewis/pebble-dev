#include <pebble.h>

#define INVERT false // used for bulding inverted version

static Window *s_window;
static BitmapLayer *s_past_bitmap_layer, *s_future_bitmap_layer, *s_mask_layer, *s_present_bitmap_layer;
static GBitmap *s_past_gbitmap, *s_future_gbitmap, *s_mask_gbitmap, *s_present_gbitmap;
static TextLayer *s_hour_layer, *s_minute_layer;

// TODO: Compat layer?
#if INVERT
static InverterLayer *invert_layer;
#endif

static TextLayer* create_text_layer(GRect location, GColor colour, GColor background, int res_id, GTextAlignment alignment) {
	TextLayer *layer = text_layer_create(location);
	text_layer_set_text_color(layer, colour);
	text_layer_set_background_color(layer, background);
	text_layer_set_font(layer, fonts_load_custom_font(resource_get_handle(res_id)));
	text_layer_set_text_alignment(layer, alignment);

	return layer;
}

static void tick_handler(struct tm *t, TimeUnits units_changed) {
  int last = 0;
  int next = 0;
  int hours = t->tm_hour;
  int mins = t->tm_min;

  static char hour_buff[16];
  static char minute_buff[16];

  // Handle clock wrapping
  if (hours == 0) {
    last = 11;
    next = 1;
  } else if (hours == 23) {
    last = 22;
    next = 0;
  } else {
    last = hours - 1;
    next = hours + 1;
  }

  // TODO: %02d for leading zero?
  if (last >= 10 && hours >= 10 && next < 10) {
    snprintf(hour_buff, 16 * sizeof(char), "%d %d 0%d", last, hours, next);
  } else if (last >= 10 && hours < 10 && next >= 10) {
    snprintf(hour_buff, 16 * sizeof(char), "%d 0%d %d", last, hours, next);
  } else if (last >= 10 && hours < 10 && next < 10) {
    snprintf(hour_buff, 16 * sizeof(char), "%d 0%d 0%d", last, hours, next);
  } else if (last < 10 && hours >= 10 && next >= 10) {
    snprintf(hour_buff, 16 * sizeof(char), "0%d %d %d", last, hours, next);
  } else if (last < 10 && hours >= 10 && next < 10) {
    snprintf(hour_buff, 16 * sizeof(char), "0%d %d 0%d", last, hours, next);
  } else if (last < 10 && hours < 10 && next >= 10) {
    snprintf(hour_buff, 16 * sizeof(char), "0%d 0%d %d", last, hours, next);
  } else if (last < 10 && hours < 10 && next < 10) {
    snprintf(hour_buff, 16 * sizeof(char), "0%d 0%d 0%d", last, hours, next);
  } else {
    snprintf(hour_buff, 16 * sizeof(char), "%d %d %d", last, hours, next);
  }

  text_layer_set_text(s_hour_layer, hour_buff);

  last = 0;
  next = 0;

  // Handle minute-clock wrapping
  if (mins == 0) {
    last = 59;
    next = 1;
  } else if (mins == 59) {
    last = 58;
    next = 0;
  } else {
    last = mins - 1;
    next = mins + 1;
  }

  if (last >= 10 && mins >= 10 && next < 10) {
    snprintf(minute_buff, 16 * sizeof(char), "%d %d 0%d", last, mins, next);
  } else if (last >= 10 && mins < 10 && next >= 10) {
    snprintf(minute_buff, 16 * sizeof(char), "%d 0%d %d", last, mins, next);
  } else if (last >= 10 && mins < 10 && next < 10) {
    snprintf(minute_buff, 16 * sizeof(char), "%d 0%d 0%d", last, mins, next);
  } else if (last < 10 && mins >= 10 && next >= 10) {
    snprintf(minute_buff, 16 * sizeof(char), "0%d %d %d", last, mins, next);
  } else if (last < 10 && mins >= 10 && next < 10) {
    snprintf(minute_buff, 16 * sizeof(char), "0%d %d 0%d", last, mins, next);
  } else if (last < 10 && mins < 10 && next >= 10) {
    snprintf(minute_buff, 16 * sizeof(char), "0%d 0%d %d", last, mins, next);
  } else if (last < 10 && mins < 10 && next < 10) {
    snprintf(minute_buff, 16 * sizeof(char), "0%d 0%d 0%d", last, mins, next);
  } else {
    snprintf(minute_buff, 16 * sizeof(char), "%d %d %d", last, mins, next);
  }

  text_layer_set_text(s_minute_layer, minute_buff);
}


static void window_load(Window *window) {
  s_past_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_PAST);
  s_present_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_PRESENT);
  s_future_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_FUTURE);
  s_mask_gbitmap = gbitmap_create_with_resource(RESOURCE_ID_MASK);

  s_past_bitmap_layer = bitmap_layer_create(GRect(-3, 0, 33, 168));
  bitmap_layer_set_bitmap(s_past_bitmap_layer, s_past_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_past_bitmap_layer));

  s_present_bitmap_layer = bitmap_layer_create(GRect(55, 0, 33, 168));
  bitmap_layer_set_bitmap(s_present_bitmap_layer, s_present_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_present_bitmap_layer));

  s_future_bitmap_layer = bitmap_layer_create(GRect(112, 0, 33, 168));
  bitmap_layer_set_bitmap(s_future_bitmap_layer, s_future_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_future_bitmap_layer));

  s_hour_layer = create_text_layer(GRect(0, 5, 144, 60), GColorBlack, GColorClear, RESOURCE_ID_FONT_MEGAFONT_22, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));

  s_minute_layer = create_text_layer(GRect(0, 25, 144, 60), GColorBlack, GColorClear, RESOURCE_ID_FONT_MEGAFONT_22, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_minute_layer));

  s_mask_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(s_mask_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_mask_layer, s_mask_gbitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_mask_layer));

#if INVERT
	invert_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(invert_layer));
#endif
}

static void window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);

#if INVERT
  inverter_layer_destroy(invert_layer);
#endif

  bitmap_layer_destroy(s_past_bitmap_layer);
  bitmap_layer_destroy(s_future_bitmap_layer);
  bitmap_layer_destroy(s_mask_layer);
  bitmap_layer_destroy(s_present_bitmap_layer);

  gbitmap_destroy(s_past_gbitmap);
  gbitmap_destroy(s_future_gbitmap);
  gbitmap_destroy(s_mask_gbitmap);
  gbitmap_destroy(s_present_gbitmap);

  window_destroy(window);
  s_window = NULL;
}

void main_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);

  time_t now = time(NULL);
  struct tm *time_now = localtime(&now);
  tick_handler(time_now, MINUTE_UNIT);
}
