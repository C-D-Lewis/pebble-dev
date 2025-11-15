#include "main_window.h"

static Window *s_window;
static BitmapLayer *s_image_layer;
static TextLayer
  *s_status_label_layer,
  *s_status_value_layer,
  *s_next_action_layer,
  *s_started_layer,
  *s_remain_label_layer,
  *s_remain_days_layer;
static GBitmap *s_image_bitmap;

static void update_data() {
  gbitmap_destroy(s_image_bitmap);

  bool is_enabled = data_get_wakeup_id() != DATA_EMPTY;
  s_image_bitmap = gbitmap_create_with_resource(is_enabled ? RESOURCE_ID_AWAKE : RESOURCE_ID_ASLEEP);

  bitmap_layer_set_bitmap(s_image_layer, s_image_bitmap);
  text_layer_set_text(s_status_value_layer, is_enabled ? "AWAKE" : "ASLEEP");

  // Next action?
  if (!is_enabled) {
    text_layer_set_text(s_next_action_layer, "Not monitoring battery");
    text_layer_set_text(s_started_layer, "");
    text_layer_set_text(s_remain_label_layer, "");
    text_layer_set_text(s_remain_days_layer, "");
  } else {
    time_t wakeup_ts;
    wakeup_query(data_get_wakeup_id(), &wakeup_ts);

    static char s_fmt_wakeup_buff[16];
    util_fmt_time(wakeup_ts, &s_fmt_wakeup_buff[0], sizeof(s_fmt_wakeup_buff));
    static char s_action_buff[32];
    snprintf(s_action_buff, sizeof(s_action_buff), "Next sample %s", &s_fmt_wakeup_buff[0]);
    text_layer_set_text(s_next_action_layer, s_action_buff);

    static char s_fmt_started_buff[16];
    util_fmt_time_ago(data_get_discharge_start_time(), &s_fmt_started_buff[0], sizeof(s_fmt_started_buff));
    static char s_started_buff[32];
    snprintf(s_started_buff, sizeof(s_started_buff), "Started %s", &s_fmt_started_buff[0]);
    text_layer_set_text(s_started_layer, s_started_buff);

    text_layer_set_text(s_remain_label_layer, "Est. remaining");

    const int days_remaining = data_calculate_days_remaining();
    if (days_remaining == DATA_EMPTY) {
      text_layer_set_text(s_remain_days_layer, "-");
    } else {
      static char s_remaining_buff[16];
      snprintf(s_remaining_buff, sizeof(s_remaining_buff), "%d days", days_remaining);
      text_layer_set_text(s_remain_days_layer, s_remaining_buff);
    }
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool should_enable = data_get_wakeup_id() == DATA_EMPTY;

  if (should_enable) {
    data_prepare();
    data_sample_now();
    wakeup_schedule_next();

    vibes_double_pulse();
  } else {
    wakeup_unschedule();
  }

  update_data();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  stat_window_push();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  GFont sys_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  GFont sys_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont sys_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);

  s_image_layer = bitmap_layer_create(GRect(5, 5, 64, 64));
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_image_layer));

  s_status_label_layer = util_make_text_layer(GRect(72, 10, WIDTH - 75, 100), sys_18);
  text_layer_set_text(s_status_label_layer, "Muninn is");
  layer_add_child(root_layer, text_layer_get_layer(s_status_label_layer));

  s_status_value_layer = util_make_text_layer(GRect(71, 22, WIDTH - 75, 100), sys_28_bold);
  layer_add_child(root_layer, text_layer_get_layer(s_status_value_layer));

  s_next_action_layer = util_make_text_layer(GRect(5, 65, WIDTH - 10, 100), sys_18);
  layer_add_child(root_layer, text_layer_get_layer(s_next_action_layer));

  s_started_layer = util_make_text_layer(GRect(5, 83, WIDTH - 10, 100), sys_18);
  layer_add_child(root_layer, text_layer_get_layer(s_started_layer));

  s_remain_label_layer = util_make_text_layer(GRect(5, 105, WIDTH - 10, 100), sys_18_bold);
  layer_add_child(root_layer, text_layer_get_layer(s_remain_label_layer));

  s_remain_days_layer = util_make_text_layer(GRect(5, 117, WIDTH - 10, 100), sys_28_bold);
  layer_add_child(root_layer, text_layer_get_layer(s_remain_days_layer));

  update_data();
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_image_layer);
  text_layer_destroy(s_status_label_layer);
  text_layer_destroy(s_status_value_layer);
  text_layer_destroy(s_next_action_layer);
  text_layer_destroy(s_started_layer);
  text_layer_destroy(s_remain_label_layer);
  text_layer_destroy(s_remain_days_layer);

  gbitmap_destroy(s_image_bitmap);

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
    window_set_click_config_provider(s_window, click_config_provider);
  }

  window_stack_push(s_window, true);
}
