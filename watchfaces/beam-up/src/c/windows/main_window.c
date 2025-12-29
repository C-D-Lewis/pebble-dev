
#include "main_window.h"

#define NUM_CHARS 5

#define DIGIT_Y scalable_y_pp(315, 325)
#define SECONDS_Y_OFFSET scalable_y(625)
#define SECONDS_HEIGHT scalable_y(30)
#define BEAM_X scalable_x(97)
#define BEAM_W scalable_x(208)
#define BEAM_H scalable_y(601)
#define DIGIT_SIZE GSize(scalable_x(347), scalable_y(357))

// Necessary to center
#define HOURS_TENS_X  PBL_IF_ROUND_ELSE(scalable_x(28),  scalable_x(-90))
#define HOURS_UNITS_X PBL_IF_ROUND_ELSE(scalable_x(217), scalable_x(146))
#define MINS_TENS_X   PBL_IF_ROUND_ELSE(scalable_x(450), scalable_x(438))
#define MINS_UNITS_X  PBL_IF_ROUND_ELSE(scalable_x(639), scalable_x(674))
#define DIGIT_NUDGE_X scalable_x_pp(0, -20);
#define DIGIT_NUDGE_Y scalable_x_pp(0, 10);
use these!

static Window *s_window;
static TextLayer *s_digits[NUM_CHARS], *s_date_layer;
static Layer *s_beams[NUM_CHARS - 1], *s_seconds_bar, *s_inv_layer, *s_bt_layer;

static GBitmap *s_bt_bitmap;

static char s_time_buff[NUM_CHARS + 1]; // + NULL char
static char s_date_buffer[32];
static int s_states[NUM_CHARS - 1]; // No colon in this array
static int s_states_prev[NUM_CHARS - 1];

/******************************** Digit logic *********************************/

static void update_digit_values(struct tm *tick_time) {
  strftime(
    s_time_buff,
    sizeof(s_time_buff),
    clock_is_24h_style() ? "%H:%M" : "%I:%M",
    tick_time
  );

  s_states[3] = s_time_buff[4] - '0'; // Convert to int
  s_states[2] = s_time_buff[3] - '0';
  s_states[1] = s_time_buff[1] - '0';
  s_states[0] = s_time_buff[0] - '0';

  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d", tick_time);
}

static void show_digit_values() {
  static char s_chars[5][2] = {"1", "2", ":", "3", "4"};
  for(int i = 0; i < NUM_CHARS; i++) {
    if (i != 2) {
      s_chars[i][0] = s_time_buff[i];
      text_layer_set_text(s_digits[i], DEBUG ? "0" : s_chars[i]);
    } else {
      text_layer_set_text(s_digits[i], ":");
    }
  }

  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void predict_next_change(struct tm *tick_time) {
  update_digit_values(tick_time);

  // Hour tens will change
  if (
    //09:59 --> 10:00
    ((s_states[0] == 0) && (s_states[1] == 9) && (s_states[2] == 5) && (s_states[3] == 9)) ||
    //19:59 --> 20:00
    ((s_states[0] == 1) && (s_states[1] == 9) && (s_states[2] == 5) && (s_states[3] == 9)) ||
    //23:59 --> 00:00
    ((s_states[0] == 2) && (s_states[1] == 3) && (s_states[2] == 5) && (s_states[3] == 9))
  ) {
    s_states[0]++;
  }

  // Hour units will change
  if ((s_states[2] == 5) && (s_states[3] == 9)) s_states[1]++;

  // Minute tens will change
  if (s_states[3] == 9) s_states[2]++;

  // Minute unit always changes
  s_states[3]++;
}

/********************************* Animation **********************************/

static void safe_animation_schedule(Animation *animation) {
  if (animation) animation_schedule(animation);
}

static Animation* animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  const bool do_animations = data_get_boolean_setting(DataKeyAnimations);
  if (!do_animations) return NULL;

  // Sleeping?
  if (do_animations && data_get_boolean_setting(DataKeySleep)) {
    const time_t now = time(NULL);
    const struct tm *tm_now = localtime(&now);
    const int hours = tm_now->tm_hour;

    if (hours >= 0 && hours < 6) return NULL;
  }

  PropertyAnimation *prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  return anim;
}

static void animate_beams(struct tm *tick_time) {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));

  const int seconds = tick_time->tm_sec;

  if (data_get_boolean_setting(DataKeyHourlyVibration)) {
    if (tick_time->tm_min == 0 && seconds == 0) {
      // Buzz buzz
      uint32_t segs[] = {200, 300, 200};
      VibePattern pattern = { .durations = segs, .num_segments = ARRAY_LENGTH(segs) };
      vibes_enqueue_custom_pattern(pattern);
    }
  }

  switch(seconds) {
    // Beams up!
    case 0: {
      update_digit_values(tick_time);
      show_digit_values();

      static Animation *anims[9];
      Layer *layer_ptr;

      // Animate stuff back into place
      if ((s_states[0] != s_states_prev[0]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[0]);
        anims[0] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(HOURS_TENS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          100
        );
        anims[1] = animate_layer(
          s_beams[0],
          layer_get_frame(s_beams[0]),
          GRect(HOURS_TENS_X + BEAM_X, 0, BEAM_W, 0),
          400,
          500
        );
        s_states_prev[0] = s_states[0];
      }
      if ((s_states[1] != s_states_prev[1]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[1]);
        anims[2] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(HOURS_UNITS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          100
        );
        anims[3] = animate_layer(
          s_beams[1],
          layer_get_frame(s_beams[1]),
          GRect(HOURS_UNITS_X + BEAM_X, 0, BEAM_W, 0),
          400,
          500
        );
        s_states_prev[1] = s_states[1];
      }
      if ((s_states[2] != s_states_prev[2]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[3]);
        anims[4] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(MINS_TENS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          100
        );
        anims[5] = animate_layer(
          s_beams[2],
          layer_get_frame(s_beams[2]),
          GRect(MINS_TENS_X + BEAM_X, 0, BEAM_W, 0),
          400,
          500
        );
        s_states_prev[2] = s_states[2];
      }
      if ((s_states[3] != s_states_prev[3]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[4]);
        anims[6] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(MINS_UNITS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          100
        );
        anims[7] = animate_layer(
          s_beams[3],
          layer_get_frame(s_beams[3]),
          GRect(MINS_UNITS_X + BEAM_X, 0, BEAM_W, 0),
          400,
          500
        );
        s_states_prev[3] = s_states[3];
      }

      // Bottom surface down
      anims[8] = animate_layer(
        s_seconds_bar,
        layer_get_frame(s_seconds_bar),
        GRect(0, SECONDS_Y_OFFSET, 0, SECONDS_HEIGHT),
        500,
        500
      );

      Animation *spawn = animation_spawn_create_from_array((Animation**)&anims, 9);
      safe_animation_schedule(spawn);
    } break;

    // Fix 'stuck' animations
    case 2:
      layer_set_frame(
        text_layer_get_layer(s_digits[0]),
        GRect(HOURS_TENS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h)
      );
      layer_set_frame(
        text_layer_get_layer(s_digits[1]),
        GRect(HOURS_UNITS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h)
      );
      layer_set_frame(
        text_layer_get_layer(s_digits[3]),
        GRect(MINS_TENS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h)
      );
      layer_set_frame(
        text_layer_get_layer(s_digits[4]),
        GRect(MINS_UNITS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h)
      );

      layer_set_frame(s_beams[0], GRect(HOURS_TENS_X + BEAM_X, 0, BEAM_W, 0));
      layer_set_frame(s_beams[1], GRect(HOURS_UNITS_X + BEAM_X, 0, BEAM_W, 0));
      layer_set_frame(s_beams[2], GRect(MINS_TENS_X + BEAM_X, 0, BEAM_W, 0));
      layer_set_frame(s_beams[3], GRect(MINS_UNITS_X + BEAM_X, 0, BEAM_W, 0));

      layer_set_frame(s_seconds_bar, GRect(0, SECONDS_Y_OFFSET, 0, SECONDS_HEIGHT));
      break;

    // 15 seconds bar
    case 15:
      safe_animation_schedule(
        animate_layer(
          s_seconds_bar,
          layer_get_frame(s_seconds_bar),
          GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 4, SECONDS_HEIGHT),
          500,
          0
        )
      );
      break;

    // 30 seconds bar
    case 30:
      safe_animation_schedule(
        animate_layer(
          s_seconds_bar,
          layer_get_frame(s_seconds_bar),
          GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 2, SECONDS_HEIGHT),
          500,
          0
        )
      );
      break;

    // 45 seconds bar
    case 45:
      safe_animation_schedule(
        animate_layer(
          s_seconds_bar,
          layer_get_frame(s_seconds_bar),
          GRect(0, SECONDS_Y_OFFSET, (3 * bounds.size.w) / 4, SECONDS_HEIGHT),
          500,
          0
        )
      );
      break;

    // Complete bar
    case 58:
      safe_animation_schedule(
        animate_layer(
          s_seconds_bar,
          layer_get_frame(s_seconds_bar),
          GRect(0, SECONDS_Y_OFFSET, bounds.size.w, SECONDS_HEIGHT),
          500,
          1000
        )
      );
      break;

    // Beams down
    case 59: {
      // Predict next changes
      predict_next_change(tick_time);

      static Animation *anims[8];
      Layer *layer_ptr;

      // Schedule animations of digits and beams together
      if ((s_states[0] != s_states_prev[0]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[0]);
        anims[0] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(HOURS_TENS_X, -DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          700
        );
        anims[1] = animate_layer(
          s_beams[0],
          layer_get_frame(s_beams[0]),
          GRect(HOURS_TENS_X + BEAM_X, 0, BEAM_W, BEAM_H),
          400,
          0
        );
      }

      if ((s_states[1] != s_states_prev[1]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[1]);
        anims[2] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(HOURS_UNITS_X, -DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          700
        );
        anims[3] = animate_layer(
          s_beams[1],
          layer_get_frame(s_beams[1]),
          GRect(HOURS_UNITS_X + BEAM_X, 0, BEAM_W, BEAM_H),
          400,
          0
        );
      }

      if ((s_states[2] != s_states_prev[2]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[3]);
        anims[4] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(MINS_TENS_X, -DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          700
        );
        anims[5] = animate_layer(
          s_beams[2],
          layer_get_frame(s_beams[2]),
          GRect(MINS_TENS_X + BEAM_X, 0, BEAM_W, BEAM_H),
          400,
          0
        );
      }

      if ((s_states[3] != s_states_prev[3]) || DEBUG) {
        layer_ptr = text_layer_get_layer(s_digits[4]);
        anims[6] = animate_layer(
          layer_ptr,
          layer_get_frame(layer_ptr),
          GRect(MINS_UNITS_X, -DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h),
          200,
          700
        );
        anims[7] = animate_layer(
          s_beams[3],
          layer_get_frame(s_beams[3]),
          GRect(MINS_UNITS_X + BEAM_X, 0, BEAM_W, BEAM_H),
          400,
          0
        );
      }

      Animation *spawn = animation_spawn_create_from_array((Animation**)&anims, 8);
      safe_animation_schedule(spawn);
    } break;
  }
}

/*********************************** Window ***********************************/

static void bt_handler(bool connected) {
  if (connected) {
    layer_set_hidden(s_bt_layer, true);
  } else {
    vibes_double_pulse();
    layer_set_hidden(s_bt_layer, false);
  }
}

static void seconds_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, data_get_foreground_color());
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void inv_update_proc(Layer *layer, GContext *ctx) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);

  for(int i = 0; i < NUM_CHARS - 1; i++) {
    universal_fb_swap_colors(
      fb,
      layer_get_frame(s_beams[i]),
      data_get_foreground_color(),
      data_get_background_color()
    );
  }

  graphics_release_frame_buffer(ctx, fb);
}

static void bt_update_proc(Layer *layer, GContext *ctx) {
  if (!data_get_boolean_setting(DataKeyBTIndicator)) return;

  // Draw it white
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_bt_bitmap, gbitmap_get_bounds(s_bt_bitmap));

  // Swap to FG color
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  universal_fb_swap_colors(fb, layer_get_frame(layer), GColorWhite, data_get_foreground_color());
  graphics_release_frame_buffer(ctx, fb);
}

static void tick_handler(struct tm* tick_time, TimeUnits changed) {
  animate_beams(tick_time);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const int colon_x = HOURS_UNITS_X + 9;
  s_digits[0] = text_layer_create(GRect(HOURS_TENS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[1] = text_layer_create(GRect(HOURS_UNITS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[2] = text_layer_create(GRect(colon_x, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[3] = text_layer_create(GRect(MINS_TENS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h));
  s_digits[4] = text_layer_create(GRect(MINS_UNITS_X, DIGIT_Y, DIGIT_SIZE.w, DIGIT_SIZE.h));

  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_set_background_color(s_digits[i], GColorClear);
    text_layer_set_text_alignment(s_digits[i], GTextAlignmentRight);
    text_layer_set_font(s_digits[i], scalable_get_font(SFI_Large));
    layer_add_child(window_layer, text_layer_get_layer(s_digits[i]));
  }

  s_beams[0] = layer_create(GRect(HOURS_TENS_X + BEAM_X, 0, BEAM_W, 0));
  s_beams[1] = layer_create(GRect(HOURS_UNITS_X + BEAM_X, 0, BEAM_W, 0));
  s_beams[2] = layer_create(GRect(MINS_TENS_X + BEAM_X, 0, BEAM_W, 0));
  s_beams[3] = layer_create(GRect(MINS_UNITS_X + BEAM_X, 0, BEAM_W, 0));

  s_inv_layer = layer_create(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, BEAM_H));
  layer_set_update_proc(s_inv_layer, inv_update_proc);
  layer_add_child(window_layer, s_inv_layer);

  const int date_x = PBL_IF_ROUND_ELSE(0, scalable_x(313));
  s_date_layer = text_layer_create(
    GRect(date_x, SECONDS_Y_OFFSET, bounds.size.w - date_x, scalable_y(179))
  );
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(
    s_date_layer,
    PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight)
  );
  text_layer_set_font(s_date_layer, scalable_get_font(SFI_Small));

  s_seconds_bar = layer_create(GRect(0, SECONDS_Y_OFFSET, 0, SECONDS_HEIGHT));
  layer_set_update_proc(s_seconds_bar, seconds_update_proc);
  layer_add_child(window_layer, s_seconds_bar);

  s_bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_WHITE);

  GRect bitmap_bounds = gbitmap_get_bounds(s_bt_bitmap);
  s_bt_layer = layer_create(
    GRect(
      (bounds.size.w - bitmap_bounds.size.w) / 2,
      scalable_y(833),
      bitmap_bounds.size.w,
      bitmap_bounds.size.h
    )
  );
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(window_layer, s_bt_layer);
}

static void window_unload(Window *window) {
  for(int i = 0; i < NUM_CHARS; i++) text_layer_destroy(s_digits[i]);
  text_layer_destroy(s_date_layer);

  for(int i = 0; i < NUM_CHARS - 1; i++) layer_destroy(s_beams[i]);
  layer_destroy(s_seconds_bar);

  layer_destroy(s_bt_layer);
  gbitmap_destroy(s_bt_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  // Make sure the face is not blank
  time_t temp = time(NULL);
  struct tm *time_now = localtime(&temp);
  update_digit_values(time_now);
  show_digit_values();

  // Stop 'all change' on first minute
  for(int i = 0; i < NUM_CHARS - 1; i++) {
    s_states_prev[i] = s_states[i];
  }

  main_window_reload_config(false);
}

static void reload_config() {
  Layer *window_layer = window_get_root_layer(s_window);

  // Services
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(
    data_get_boolean_setting(DataKeyAnimations) ? SECOND_UNIT : MINUTE_UNIT,
    tick_handler
  );
  if (data_get_boolean_setting(DataKeyBTIndicator)) {
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = bt_handler
    });
  } else {
    connection_service_unsubscribe();
  }  

  // BT layer
  if (data_get_boolean_setting(DataKeyBTIndicator)) {
    layer_set_hidden(s_bt_layer, connection_service_peek_pebble_app_connection());
  } else {
    layer_set_hidden(s_bt_layer, true);
  }  

  for(int i = 0; i < 4; i++) {
    if (data_get_boolean_setting(DataKeyAnimations)) {
      layer_add_child(window_layer, s_beams[i]);
      layer_add_child(window_layer, s_seconds_bar);
    } else {
      // Remove animation layers in case they are in progress
      layer_remove_from_parent(s_beams[i]);
      layer_remove_from_parent(s_seconds_bar);
    }
  }

  // Colors
  window_set_background_color(s_window, data_get_background_color());
  for(int i = 0; i < NUM_CHARS; i++) {
    text_layer_set_text_color(s_digits[i], data_get_foreground_color());
  }
  text_layer_set_text_color(s_date_layer, data_get_foreground_color());

  // Show date
  if (data_get_boolean_setting(DataKeyDate)) {
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  } else {
    layer_remove_from_parent(text_layer_get_layer(s_date_layer));
  }

  layer_mark_dirty(s_bt_layer);
  layer_mark_dirty(s_inv_layer);  

  // Init seconds bar
  GRect bounds = layer_get_bounds(window_layer);
  const time_t temp = time(NULL);
  const struct tm *time_now = localtime(&temp);
  const int seconds = time_now->tm_sec;

  if (seconds >= 15 && seconds < 30) {
    safe_animation_schedule(
      animate_layer(
        s_seconds_bar,
        layer_get_frame(s_seconds_bar),
        GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 4, SECONDS_HEIGHT),
        500,
        0
      )
    );
  } else if (seconds >= 30 && seconds < 45) {
    safe_animation_schedule(
      animate_layer(
        s_seconds_bar,
        layer_get_frame(s_seconds_bar),
        GRect(0, SECONDS_Y_OFFSET, bounds.size.w / 2, SECONDS_HEIGHT),
        500,
        0
      )
    );
  } else if (seconds >= 45 && seconds < 58) {
    safe_animation_schedule(
      animate_layer(
        s_seconds_bar,
        layer_get_frame(s_seconds_bar),
        GRect(0, SECONDS_Y_OFFSET, (3 * bounds.size.w) / 4, SECONDS_HEIGHT),
        500,
        0
      )
    );
  } else if (seconds >= 58) {
    safe_animation_schedule(
      animate_layer(
        s_seconds_bar,
        layer_get_frame(s_seconds_bar),
        GRect(0, SECONDS_Y_OFFSET, bounds.size.w, SECONDS_HEIGHT),
        500,
        0
      )
    );
  }
}

static void reload_config_handler(void *context) {
  reload_config();
}

void main_window_reload_config(bool delay) {
  if (delay) {
    // Avoid weird things happening mid beaming up
    app_timer_register(2000, reload_config_handler, NULL);
  } else {
    reload_config();
  }
}
