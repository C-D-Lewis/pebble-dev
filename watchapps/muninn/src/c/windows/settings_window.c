#include "settings_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

static bool s_reset_confirm;

typedef enum {
  MI_VIBE_ON_SAMPLE,
  MI_CUSTOM_ALERT_LEVEL,
  MI_PUSH_TIMELINE_PINS,
  MI_ELEVATED_RATE_ALERT,
  MI_ONE_DAY_ALERT,

  MI_BATTERY_TIPS,
  MI_DELETE_ALL_DATA,
  MI_VERSION,

  MI_MAX,
} MenuItems;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Alert level detail
  const int alert_level = data_get_custom_alert_level();
  const bool alert_disabled = alert_level == AL_OFF;
  static char s_alert_buff[24];
  if (!alert_disabled) {
    snprintf(s_alert_buff, sizeof(s_alert_buff), "Notifying near %d%%", alert_level);
  }

  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Vibrate on sample",
        data_get_vibe_on_sample() ? "Enabled" : "Disabled"
      );
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Custom alert",
        alert_disabled ? "Disabled" : s_alert_buff
      );
      break;
    case MI_PUSH_TIMELINE_PINS:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Timeline pins",
        data_get_push_timeline_pins() ? "Enabled" : "Disabled"
      );
      break;
    case MI_ELEVATED_RATE_ALERT:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "High drain alert",
        data_get_elevated_rate_alert() ? "Enabled" : "Disabled"
      );
      break;
    case MI_ONE_DAY_ALERT:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "One day left alert",
        data_get_one_day_alert() ? "Enabled" : "Disabled"
      );
      break;
    case MI_BATTERY_TIPS:
      util_menu_cell_draw(ctx, cell_layer, "Battery tips", NULL);
      break;
    case MI_DELETE_ALL_DATA:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Delete all data",
        s_reset_confirm ? "Tap again to confirm" : NULL
      );
      break;
    case MI_VERSION: {
      static char s_v_buff[16];
      snprintf(s_v_buff, sizeof(s_v_buff), "Version %s", VERSION);
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "About Muninn",
        s_v_buff
      );
      break;
    }
    default: break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
    case MI_CUSTOM_ALERT_LEVEL:
    case MI_PUSH_TIMELINE_PINS:
    case MI_ELEVATED_RATE_ALERT:
    case MI_ONE_DAY_ALERT:
    case MI_VERSION:
      return ROW_HEIGHT_LARGE;
    case MI_DELETE_ALL_DATA:
      return s_reset_confirm ? ROW_HEIGHT_LARGE : ROW_HEIGHT_SMALL;
    default:
      return ROW_HEIGHT_SMALL;
  }
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    // Options
    case MI_VIBE_ON_SAMPLE:
      data_set_vibe_on_sample(!data_get_vibe_on_sample());
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      data_cycle_custom_alert_level();
      break;
    case MI_PUSH_TIMELINE_PINS: {
      const bool now_enabled = !data_get_push_timeline_pins();
      data_set_push_timeline_pins(now_enabled);

      if (now_enabled) comm_push_timeline_pins();
    } break;
    case MI_ELEVATED_RATE_ALERT:
      data_set_elevated_rate_alert(!data_get_elevated_rate_alert());
      break;
    case MI_ONE_DAY_ALERT:
      data_set_one_day_alert(!data_get_one_day_alert());
      break;

    // Other
    case MI_BATTERY_TIPS:
      message_window_push(MSG_TIPS, false, false);
      break;
    case MI_DELETE_ALL_DATA:
      if (s_reset_confirm) {
        data_reset_all();
        vibes_double_pulse();
        window_stack_pop_all(true);
      } else {
        vibes_long_pulse();
      }

      s_reset_confirm = !s_reset_confirm;
      break;
    case MI_VERSION:
      message_window_push(MSG_INFORMATION, false, false);
      break;
    default: break;
  }

  menu_layer_reload_data(s_menu_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
    .select_click = (MenuLayerSelectCallback)select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  s_reset_confirm = false;

  window_destroy(window);
  s_window = NULL;
}

void settings_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
