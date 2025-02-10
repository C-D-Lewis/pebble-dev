#include <pebble.h>

#include "config.h"

#include "modules/data.h"
#include "modules/comm.h"

#include <pebble-pge-simple/pebble-pge-simple.h>
#include <pebble-isometric/pebble-isometric.h>
#include <pebble-universal-fb/pebble-universal-fb.h>

#include "drawable/segment.h"
#include "drawable/digit.h"

static Digit *s_digits[4];

static bool s_initd;

static bool digits_are_animating() {
  for(int i = 0; i < 4; i++) {
    if(digit_is_animating(s_digits[i])) {
      return true;
    }
  }
  return false;
}

#if defined(PBL_BW)
static void filter_gaps(GContext *ctx) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  GRect bounds = gbitmap_get_bounds(fb);

  // For all origins
  for(int y = 0; y < bounds.size.h; y++) {
    if(y >= bounds.size.h - 5) {
      // Don't go out of bounds near the end
      continue;
    }

    GBitmapDataRowInfo info1 = gbitmap_get_data_row_info(fb, y);
    for(int x = info1.min_x; x <= info1.max_x; x++) {
      // Look for w w b w w pattern
      GColor p1 = universal_fb_get_pixel_color(info1, bounds, GPoint(x, y));
      GBitmapDataRowInfo info2 = gbitmap_get_data_row_info(fb, y + 1);
      GColor p2 = universal_fb_get_pixel_color(info2, bounds, GPoint(x, y + 1));
      GBitmapDataRowInfo info3 = gbitmap_get_data_row_info(fb, y + 2);
      GColor p3 = universal_fb_get_pixel_color(info3, bounds, GPoint(x, y + 2));
      GBitmapDataRowInfo info4 = gbitmap_get_data_row_info(fb, y + 3);
      GColor p4 = universal_fb_get_pixel_color(info4, bounds, GPoint(x, y + 3));
      GBitmapDataRowInfo info5 = gbitmap_get_data_row_info(fb, y + 4);
      GColor p5 = universal_fb_get_pixel_color(info5, bounds, GPoint(x, y + 4));
      if(gcolor_equal(p1, GColorWhite)
      && gcolor_equal(p2, GColorWhite)
      && gcolor_equal(p3, GColorBlack)
      && gcolor_equal(p4, GColorWhite)
      && gcolor_equal(p5, GColorWhite)) {
        // universal_fb_set_pixel_color(info3, bounds, GPoint(x - 1, y + 2), GColorWhite);
        universal_fb_set_pixel_color(info3, bounds, GPoint(x, y + 2), GColorWhite);
      }
    }
  }

  graphics_release_frame_buffer(ctx, fb);
}
#endif

static void pge_logic() {
  for(int i = 0; i < 4; i++) {
    digit_logic(s_digits[i]);
  }
}

static void pge_render(GContext *ctx) {
#ifdef BENCHMARK
  uint16_t start = time_ms(NULL, NULL);
#endif

  isometric_begin(ctx);
  for(int i = 0; i < 4; i++) {
    digit_render(s_digits[i]);
  }
  isometric_finish(ctx);

#if defined(PBL_BW)
  filter_gaps(ctx);
#endif

  if(!digits_are_animating()) {
    pge_pause();
    if(!s_initd) {
      // Any further interruptions will cause fast-forwarding
      s_initd = true;
    }
  }

#ifdef BENCHMARK
  uint16_t finish = time_ms(NULL, NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Frame time: %d", (int)finish - start);
#endif
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  int hours = tick_time->tm_hour;
  int mins = tick_time->tm_min;

  if(!clock_is_24h_style() && hours > 12) {
    hours -= 12;
  }

  if(hours < 10) {
    digit_set_value(s_digits[0], 0);
  } else {
    digit_set_value(s_digits[0], hours / 10);
  }
  digit_set_value(s_digits[1], hours % 10);

  if(mins < 10) {
    digit_set_value(s_digits[2], 0);
  } else {
    digit_set_value(s_digits[2], mins / 10);
  }
  digit_set_value(s_digits[3], mins % 10);

  // Smooth transition
  if(data_get_animations()) {
    pge_resume();
  } else {
    // Instant transition
    while(digits_are_animating()) {
      pge_logic();
    }
    pge_manual_advance();
  }
}

static void fix_handler(void *context) {
  // Render new state
  pge_manual_advance();
}

static void focus_handler(bool now_in_focus) {
  if(now_in_focus && s_initd) {
    // Prevent bad rendering after notifications
    while(digits_are_animating()) {
      pge_logic();
    }

    app_timer_register(500, fix_handler, NULL);
  }
}

static void bt_handler(bool connected) {
#if defined(PBL_COLOR)
  // Gray digits
  if(connected) {
    for(int i = 0; i < 4; i++) {
      digit_set_colors(s_digits[i], data_get_color(ColorSides), data_get_color(ColorFace));
    }
  } else {
    for(int i = 0; i < 4; i++) {
      digit_set_colors(s_digits[i], GColorDarkGray, GColorLightGray);
    }
  }
  pge_manual_advance();
#endif

  if(!connected) {
    vibes_double_pulse();
  }
}

void pge_init() {
  s_initd = false;

  data_init();

  const int buffer_size = 64;
  comm_init(buffer_size, buffer_size);

  s_digits[0] = digit_create(GPoint(-HOURS_OFFSET, 0), 0);
  s_digits[1] = digit_create(GPoint(-HOURS_OFFSET + (5 * BLOCK_SIZE), 0), 0);
  s_digits[2] = digit_create(GPoint(MINS_OFFSET, 7 * BLOCK_SIZE), 0);
  s_digits[3] = digit_create(GPoint(MINS_OFFSET + (5 * BLOCK_SIZE), 7 * BLOCK_SIZE), 0);

  isometric_set_projection_offset(PROJECTION_OFFSET);
  pge_set_framerate(FRAME_RATE_HIGH);
  pge_begin(pge_logic, pge_render, NULL);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  tick_handler(t, MINUTE_UNIT);

  main_reload_config();

  // Fast forward - save power when lots of notifications
  while(digits_are_animating()) {
    pge_logic();
  }
  app_focus_service_subscribe(focus_handler);

  // If no animations
  if(!data_get_animations()) {
    pge_manual_advance();

    // Don't animate after this
    pge_pause();
  }
}

void pge_deinit() {
  pge_finish();

  for(int i = 0; i < 4; i++) {
    digit_destroy(s_digits[i]);
  }
}

void main_reload_config() {
  // Must be before BT handle overwrites the BG color!
  #if defined(PBL_COLOR)
    pge_set_background_color(data_get_color(ColorBackground));
  #elif defined(PBL_BW)
    pge_set_background_color(GColorBlack);
  #endif

  // If bluetooth alert enabled
  if(data_get_bluetooth_alert()) {
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bt_handler
    });
    bt_handler(connection_service_peek_pebble_app_connection());
  } else {
    connection_service_unsubscribe();
  }

  for(int i = 0; i < 4; i++) {
    digit_set_colors(s_digits[i], data_get_color(ColorSides), data_get_color(ColorFace));
  }

  pge_manual_advance();
}
