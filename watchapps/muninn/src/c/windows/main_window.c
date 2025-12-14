#include "main_window.h"

// Not scaled
#if defined(PBL_PLATFORM_EMERY)
  #define MASCOT_SIZE 66
  #define EYE_RECT GRect(58, 10, 3, 3)
  #define DIV_W 2
  #define ROW_1_SUBTITLE "   Days left       Est. %/day"
  #define BRAID_H 18
  #define ICON_SIZE 28
#else
  #define MASCOT_SIZE 48
  #define EYE_RECT GRect(41, 7, 4, 4)
  #define DIV_W 1
  #define ROW_1_SUBTITLE "    Days          Est. %/d"
  #define BRAID_H 14
  #define ICON_SIZE 24
#endif

#define ACTION_BAR_W scalable_x(70)
#define BRAID_Y scalable_y_pp(415, 425)
#define HINT_W scalable_x(70)
#define HINT_H scalable_y(200)
#define ROW_DIV_Y scalable_y(810)

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer
  *s_status_label_layer,
  *s_status_value_layer,
  *s_desc_layer,
  *s_battery_layer,
  *s_reading_layer,
  *s_row_1_subtitle_layer,
  *s_remaining_layer,
  *s_rate_layer,
  *s_hint_layer;
static BitmapLayer
  *s_mascot_layer,
  *s_battery_bmp_layer,
  *s_reading_bmp_layer,
  *s_remaining_bmp_layer,
  *s_rate_bmp_layer;
static GBitmap
  *s_mascot_bitmap,
  *s_battery_bitmap,
  *s_reading_bitmap,
  *s_remaining_bitmap,
  *s_rate_bitmap,
  *s_braid_bitmap;

static AppTimer *s_blink_timer;
static bool s_is_blinking;
static int s_blink_budget;

static void schedule_blink();

static void unblink_handler(void *context) {
  s_is_blinking = false;
  layer_mark_dirty(s_canvas_layer);

  schedule_blink();
}

static void blink_handler(void *context) {
  s_is_blinking = true;
  layer_mark_dirty(s_canvas_layer);

  s_blink_timer = app_timer_register(500, unblink_handler, NULL);
}

static void schedule_blink() {
  // Only blink a few times in case app is left open
  if (s_blink_budget == 0) {
    s_blink_timer = NULL;
    return;
  }

  s_blink_budget--;
  s_blink_timer = app_timer_register(700 + (rand() % 3000), blink_handler, NULL);
}

static void cancel_blink() {
  if (s_blink_timer) {
    app_timer_cancel(s_blink_timer);
    s_blink_timer = NULL;
  }

  s_is_blinking = false;
  layer_mark_dirty(s_canvas_layer);
}

static void update_data() {
  if (s_mascot_bitmap) {
    gbitmap_destroy(s_mascot_bitmap);
    s_mascot_bitmap = NULL;
  }
  if (s_battery_bitmap) {
    gbitmap_destroy(s_battery_bitmap);
    s_battery_bitmap = NULL;
  }

  time_t wakeup_ts;
  const int wakeup_id = data_get_wakeup_id();
  const bool is_enabled = util_is_valid(wakeup_id);
  const bool found = wakeup_query(wakeup_id, &wakeup_ts);
  if (is_enabled && !found) {
    data_set_error("Scheduled wakeup was not found");
  }

  s_mascot_bitmap = gbitmap_create_with_resource(
    is_enabled ? RESOURCE_ID_AWAKE : RESOURCE_ID_ASLEEP
  );
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");

  cancel_blink();

  // Battery now
  BatteryChargeState state = battery_state_service_peek();
  const int charge_percent = state.charge_percent;
  s_battery_bitmap = gbitmap_create_with_resource(util_get_battery_resource_id(charge_percent));
  bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
  static char s_battery_buff[16];
  snprintf(s_battery_buff, sizeof(s_battery_buff), "%d%%", charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buff);

  // Current status
  text_layer_set_text(s_desc_layer, util_get_status_string());

  // Sometimes
  if (!is_enabled) {
    text_layer_set_text(s_reading_layer, " -");
    text_layer_set_text(s_remaining_layer, " -");
    text_layer_set_text(s_rate_layer, " -");

    layer_set_hidden(text_layer_get_layer(s_hint_layer), false);
  } else {
    layer_set_hidden(text_layer_get_layer(s_hint_layer), true);
    schedule_blink();

    // Days remaining
    const int days_remaining = data_calculate_days_remaining();
    if (!util_is_valid(days_remaining)) {
      text_layer_set_text(s_remaining_layer, " -");
    } else {
      static char s_remaining_buff[16];
      snprintf(s_remaining_buff, sizeof(s_remaining_buff), "%d", days_remaining);
      text_layer_set_text(s_remaining_layer, s_remaining_buff);
    }

    // Rate per day
    const int rate = data_calculate_avg_discharge_rate();
    if (!util_is_valid(rate)) {
      text_layer_set_text(s_rate_layer, " -");
    } else {
      static char s_rate_buff[16];
      snprintf(s_rate_buff, sizeof(s_rate_buff), "%d", rate);
      text_layer_set_text(s_rate_layer, s_rate_buff);
    }

    // Next reading
    static char s_wakeup_buff[8];
    util_fmt_time(wakeup_ts, &s_wakeup_buff[0], sizeof(s_wakeup_buff));
    text_layer_set_text(s_reading_layer, s_wakeup_buff);
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Divider braid
  const GRect braid_rect = GRect(0, BRAID_Y, DISPLAY_W, BRAID_H);
  graphics_draw_bitmap_in_rect(ctx, s_braid_bitmap, braid_rect);

  // Actions BG
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorJazzberryJam, GColorLightGray));
  GRect actions_rect = GRect(DISPLAY_W - ACTION_BAR_W, 0, ACTION_BAR_W, DISPLAY_H);
  graphics_fill_rect(ctx, actions_rect, 0, GCornerNone);

  const int hint_x = DISPLAY_W - (HINT_W / 2);

  // Enable hint
  const int enable_y = DISPLAY_H / 6 - (HINT_H / 2);
  const GRect enable_rect = GRect(hint_x, enable_y, HINT_W, HINT_H);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, enable_rect, 3, GCornersAll);
  graphics_context_set_fill_color(ctx, GColorWhite);
  const GPoint select_center = {
    .x = hint_x + (HINT_W / 2),
    .y = enable_y + (HINT_H / 2)
  };
  graphics_fill_circle(ctx, select_center, scalable_x(20));

  // Blink Muninn's eye
  if (s_is_blinking) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, EYE_RECT, 0, GCornerNone);
  }

  // Menu hint
  int menu_y = (DISPLAY_H / 2) - (HINT_H / 2);
  GRect menu_rect = GRect(hint_x, menu_y, HINT_W, HINT_H);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, menu_rect, 3, GCornersAll);

  // Row divider
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, DIV_W);
  graphics_draw_line(
    ctx,
    GPoint(0, ROW_DIV_Y),
    GPoint(DISPLAY_W - (1 * ACTION_BAR_W), ROW_DIV_Y)
  );
  const int row_div_x = (DISPLAY_W / 2) - scalable_x(80);
  const int row_div_h = scalable_y(320);
  graphics_draw_line(
    ctx,
    GPoint(row_div_x, ROW_DIV_Y),
    GPoint(row_div_x, ROW_DIV_Y - row_div_h)
  );
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  const bool should_enable = !util_is_valid(data_get_wakeup_id());
  if (should_enable) {
    s_blink_budget = 5;
    schedule_blink();

    wakeup_schedule_next();
  } else {
    wakeup_unschedule();
  }

  vibes_long_pulse();
  update_data();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  menu_window_push();
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  stat_window_push();
}

static void click_config_provider(void *context) {
  window_long_click_subscribe(BUTTON_ID_UP, 1000, up_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);

  // Debugging info
  window_long_click_subscribe(BUTTON_ID_DOWN, 3000, down_long_click_handler, NULL);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_braid_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BRAID);

  // Mascot
  s_mascot_bitmap = gbitmap_create_with_resource(RESOURCE_ID_AWAKE);
  s_mascot_layer = bitmap_layer_create(
    GRect(scalable_x(30), scalable_y(20), MASCOT_SIZE, MASCOT_SIZE)
  );
  bitmap_layer_set_compositing_mode(s_mascot_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_mascot_layer, s_mascot_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_mascot_layer));

  // On top of mascot
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  s_status_label_layer = util_make_text_layer(
    scalable_grect_pp(
      GRect(440, 20, 1000, 500),
      GRect(440, 10, 1000, 500)
    ),
    scalable_get_font(SFI_Small)
  );
  text_layer_set_text(s_status_label_layer, "Muninn is");
  layer_add_child(root_layer, text_layer_get_layer(s_status_label_layer));

  s_status_value_layer = util_make_text_layer(
    scalable_grect_pp(
      GRect(430, 90, 1000, 500),
      GRect(440, 130, 1000, 500)
    ),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  s_desc_layer = util_make_text_layer(
    scalable_grect_pp(
      GRect(0, 270, 930, 300),
      GRect(0, 280, 930, 300)
    ),
    scalable_get_font(SFI_Small)
  );
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_desc_layer));

  // Top row
  int row_x = scalable_x(50);
  int row_y = scalable_y_pp(535, 550);

  int text_icon_offset = scalable_x_pp(190, 160);
  const int row_1_text_y_offset = scalable_y_pp(-35, -15);

  s_remaining_bitmap = gbitmap_create_with_resource(RESOURCE_ID_REMAINING);
  s_remaining_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_remaining_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_remaining_bmp_layer, s_remaining_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_remaining_bmp_layer));
  s_remaining_layer = util_make_text_layer(
    GRect(row_x + text_icon_offset, row_y + row_1_text_y_offset, DISPLAY_W, 100),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_remaining_layer));

  row_x += scalable_x_pp(440, 440);

  s_rate_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RATE);
  s_rate_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_rate_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_rate_bmp_layer, s_rate_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_rate_bmp_layer));
  s_rate_layer = util_make_text_layer(
    GRect(row_x + text_icon_offset, row_y + row_1_text_y_offset, DISPLAY_W, 100),
    scalable_get_font(SFI_LargeBold)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_rate_layer));

  s_row_1_subtitle_layer = util_make_text_layer(
    GRect(2, row_y + scalable_y_pp(120, 110), DISPLAY_W - ACTION_BAR_W, 40),
    scalable_get_font(SFI_Small)
  );
  text_layer_set_text(s_row_1_subtitle_layer, ROW_1_SUBTITLE);
  layer_add_child(root_layer, text_layer_get_layer(s_row_1_subtitle_layer));

  // Bottom row
  row_x = scalable_x(20);
  row_y = scalable_y_pp(840, 850);
  text_icon_offset = scalable_x_pp(160, 150);

  const int row_2_text_y_offset = scalable_y_pp(-30, -20);

  s_battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_HIGH);
  s_battery_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_battery_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_battery_bmp_layer, s_battery_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_battery_bmp_layer));
  s_battery_layer = util_make_text_layer(
    GRect(row_x + text_icon_offset, row_y + row_2_text_y_offset, DISPLAY_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_battery_layer));

  row_x += scalable_x_pp(420, 430);

  s_reading_bitmap = gbitmap_create_with_resource(RESOURCE_ID_READING);
  s_reading_bmp_layer = bitmap_layer_create(GRect(row_x, row_y, ICON_SIZE, ICON_SIZE));
  bitmap_layer_set_compositing_mode(s_reading_bmp_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_reading_bmp_layer, s_reading_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_reading_bmp_layer));
  s_reading_layer = util_make_text_layer(
    GRect(row_x + text_icon_offset, row_y + row_2_text_y_offset, DISPLAY_W, 100),
    scalable_get_font(SFI_Medium)
  );
  layer_add_child(root_layer, text_layer_get_layer(s_reading_layer));

  // Hint for when Muninn is asleep
  const GRect hint_rect = GRect(
    2,
    BRAID_Y + BRAID_H,
    DISPLAY_W - ACTION_BAR_W - 4,
    (ROW_DIV_Y - BRAID_Y - BRAID_H)
  );
  s_hint_layer = util_make_text_layer(hint_rect, scalable_get_font(SFI_Medium));
  text_layer_set_background_color(s_hint_layer, GColorWhite);
  text_layer_set_text_alignment(s_hint_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hint_layer, "Hold the Up button to wake Muninn.");
  layer_add_child(root_layer, text_layer_get_layer(s_hint_layer));

  update_data();
}

static void window_unload(Window *window) {
  text_layer_destroy(s_status_label_layer);
  text_layer_destroy(s_status_value_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_row_1_subtitle_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_reading_layer);
  text_layer_destroy(s_remaining_layer);
  text_layer_destroy(s_rate_layer);
  text_layer_destroy(s_hint_layer);

  gbitmap_destroy(s_mascot_bitmap);
  gbitmap_destroy(s_braid_bitmap);
  gbitmap_destroy(s_battery_bitmap);
  gbitmap_destroy(s_reading_bitmap);
  gbitmap_destroy(s_remaining_bitmap);
  gbitmap_destroy(s_rate_bitmap);

  bitmap_layer_destroy(s_mascot_layer);
  bitmap_layer_destroy(s_battery_bmp_layer);
  bitmap_layer_destroy(s_reading_bmp_layer);
  bitmap_layer_destroy(s_remaining_bmp_layer);
  bitmap_layer_destroy(s_rate_bmp_layer);

  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_data();
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(s_window, click_config_provider);

    // Update stuff while app is open
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    s_blink_budget = 5;
  }

  window_stack_push(s_window, true);
}
