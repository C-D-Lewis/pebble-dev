#include "menu_window.h"

#define NUM_ITEMS 6
#define ROW_HEIGHT_SMALL 38
#define ROW_HEIGHT_LARGE 44

static Window *s_window;
static MenuLayer *s_menu_layer;

static bool s_clear_confirm;

typedef enum {
  MI_VIBE_ON_SAMPLE = 0,
  MI_CUSTOM_ALERT_LEVEL = 1,
  MI_ESTIMATE_LOG = 2,
  MI_BATTERY_TIPS = 3,
  MI_ABOUT = 4,
  MI_DELETE_ALL_DATA = 5,
  MI_MAX = 6
} MenuItems;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const int alert_level = data_get_custom_alert_level();
  const bool alert_disabled = alert_level == AL_OFF;
  static char s_alert_buff[32];
  if (!alert_disabled) {
    snprintf(s_alert_buff, sizeof(s_alert_buff), "Will notify around %d%%", alert_level);
  }

  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
      menu_cell_basic_draw(
        ctx,
        cell_layer,
        "Vibrate on sample",
        data_get_vibe_on_sample() ? "Enabled" : "Disabled",
        NULL
      );
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      menu_cell_basic_draw(
        ctx,
        cell_layer,
        "Custom threshold",
        alert_disabled ? "Disabled" : s_alert_buff,
        NULL
      );
      break;
    case MI_ESTIMATE_LOG:
      menu_cell_basic_draw(ctx, cell_layer, "Estimate log", NULL, NULL);
      break;
    case MI_BATTERY_TIPS:
      menu_cell_basic_draw(ctx, cell_layer, "Battery tips", NULL, NULL);
      break;
    case MI_ABOUT:
      menu_cell_basic_draw(ctx, cell_layer, "About", NULL, NULL);
      break;
    case MI_DELETE_ALL_DATA:
      menu_cell_basic_draw(
        ctx,
        cell_layer,
        "Delete all data",
        s_clear_confirm ? "Tap again to confirm" : NULL,
        NULL
      );
      break;
    default: break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
    case MI_CUSTOM_ALERT_LEVEL: return ROW_HEIGHT_LARGE;
    case MI_DELETE_ALL_DATA: return s_clear_confirm ? ROW_HEIGHT_LARGE : ROW_HEIGHT_SMALL;
    default: return ROW_HEIGHT_SMALL;
  }
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
      data_set_vibe_on_sample(!data_get_vibe_on_sample());
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      data_cycle_custom_alert_level();
      break;
    case MI_ESTIMATE_LOG:
      if (data_get_samples_count() == 0) {
        alert_window_push(
          RESOURCE_ID_ASLEEP,
          "No estimates yet.\n\nMuninn will begin estimating soon.",
          true,
          false
        );
      } else {
        history_window_push();
      }
      break;
    case MI_BATTERY_TIPS:
      message_window_push(MSG_TIPS);
      break;
    case MI_ABOUT:
      message_window_push(MSG_ABOUT);
      break;
    case MI_DELETE_ALL_DATA:
      if (s_clear_confirm) {
        data_reset_all();
        vibes_double_pulse();
        window_stack_pop_all(true);
      } else {
        vibes_long_pulse();
      }

      s_clear_confirm = !s_clear_confirm;
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

  s_clear_confirm = false;

  window_destroy(window);
  s_window = NULL;
}

void menu_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
