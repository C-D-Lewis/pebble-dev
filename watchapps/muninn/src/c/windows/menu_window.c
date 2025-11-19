#include "menu_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

static bool s_clear_confirm;

char *about_text = "Odin tasked Muninn with memory of the land...\n\nHe wakes every 12 hours to note the battery level.\n\nOver time, he will provide you with battery wisdom.";
char *tips_text = "Use a watchface that updates every minute.\n\nFilter notifications from very noisy apps.\n\nDisable the motion activated backlight.";

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return 6;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  const int alert_level = data_get_custom_alert_level();
  const bool alert_disabled = alert_level == AL_OFF;
  static char s_alert_buff[32];
  if (!alert_disabled) {
    snprintf(s_alert_buff, sizeof(s_alert_buff), "Will notify around %d%%", alert_level);
  }

  switch(cell_index->row) {
    case 0:
      menu_cell_basic_draw(
        ctx,
        cell_layer,
        "Vibrate on sample",
        data_get_vibe_on_sample() ? "Enabled" : "Disabled",
        NULL
      );
      break;
    case 1:
      menu_cell_basic_draw(
        ctx,
        cell_layer,
        "Custom alert",
        alert_disabled ? "Disabled" : s_alert_buff,
        NULL
      );
      break;
    case 2:
      menu_cell_basic_draw(ctx, cell_layer, "Samples log", NULL, NULL);
      break;
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, "About", NULL, NULL);
      break;
    case 4:
      menu_cell_basic_draw(ctx, cell_layer, "Battery Tips", NULL, NULL);
      break;
    case 5:
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
  return 44;
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case 0:
      data_set_vibe_on_sample(!data_get_vibe_on_sample());
      break;
    case 1:
      data_cycle_custom_alert_level();
      break;
    case 2:
      if (data_get_samples_count() == 0) {
        alert_window_push(
          RESOURCE_ID_ASLEEP,
          "No samples yet.\n\nMuninn will take a sample soon.",
          true,
          false
        );
      } else {
        history_window_push();
      }
      break;
    case 3:
      message_window_push(about_text);
      break;
    case 4:
      message_window_push(tips_text);
      break;
    case 5:
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
