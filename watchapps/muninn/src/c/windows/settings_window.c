#include "settings_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static Layer *s_header_layer;

static bool s_reset_confirm;

typedef enum {
  MI_VIBE_ON_SAMPLE,
  MI_CUSTOM_ALERT_LEVEL,
  MI_PUSH_TIMELINE_PINS,
  MI_ELEVATED_RATE_ALERT,
  MI_ONE_DAY_ALERT,
  MI_REVERSE_DATES,
#ifdef FEATURE_SYNC
  MI_AUTO_UPLOAD,
#endif

  MI_DELETE_ALL_DATA,

  MI_MAX,
} MenuItems;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  PersistData *persist_data = data_get_persist_data();

  // Alert level detail
  const int alert_level = persist_data->custom_alert_level;
  const bool alert_disabled = alert_level == AL_OFF;
  static char s_alert_buff[24];
  if (!alert_disabled) {
    snprintf(s_alert_buff, sizeof(s_alert_buff), "Notify near %d%%", alert_level);
  }

  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Vibrate on Sample",
        persist_data->vibe_on_sample ? "Enabled" : "Disabled"
      );
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Custom Alert",
        alert_disabled ? "Disabled" : s_alert_buff
      );
      break;
    case MI_PUSH_TIMELINE_PINS:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Timeline Pins",
        persist_data->push_timeline_pins ? "Enabled" : "Disabled"
      );
      break;
    case MI_ELEVATED_RATE_ALERT:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "High Drain Alert",
        persist_data->elevated_rate_alert ? "Enabled" : "Disabled"
      );
      break;
    case MI_ONE_DAY_ALERT:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "One Day Left Alert",
        persist_data->one_day_alert ? "Enabled" : "Disabled"
      );
      break;
    case MI_REVERSE_DATES:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Date Format",
        persist_data->reverse_dates ? "MM/DD" : "DD/MM"
      );
      break;
#ifdef FEATURE_SYNC
    case MI_AUTO_UPLOAD:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Daily Upload (Web)",
        persist_data->auto_upload ? "Enabled" : "Disabled"
      );
      break;
#endif
    case MI_DELETE_ALL_DATA:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Delete All Data",
        s_reset_confirm ? "ARE YOU SURE?" : NULL
      );
      break;
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
    case MI_REVERSE_DATES:
#ifdef FEATURE_SYNC
    case MI_AUTO_UPLOAD:
#endif
      return ROW_HEIGHT_LARGE;
    case MI_DELETE_ALL_DATA:
      return s_reset_confirm ? ROW_HEIGHT_LARGE : ROW_HEIGHT_SMALL;
    default:
      return ROW_HEIGHT_SMALL;
  }
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  PersistData *persist_data = data_get_persist_data();

  switch(cell_index->row) {
    // Options
    case MI_VIBE_ON_SAMPLE:
      persist_data->vibe_on_sample = !persist_data->vibe_on_sample;
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      data_cycle_custom_alert_level();
      break;
    case MI_PUSH_TIMELINE_PINS:
      persist_data->push_timeline_pins = !persist_data->push_timeline_pins;
      break;
    case MI_ELEVATED_RATE_ALERT:
      persist_data->elevated_rate_alert = !persist_data->elevated_rate_alert;
      break;
    case MI_ONE_DAY_ALERT:
      persist_data->one_day_alert = !persist_data->one_day_alert;
      break;
    case MI_REVERSE_DATES:
      persist_data->reverse_dates = !persist_data->reverse_dates;
      break;
#ifdef FEATURE_SYNC
    case MI_AUTO_UPLOAD: {
      const bool new_state = !persist_data->auto_upload;
      persist_data->auto_upload = new_state;

      if (new_state) {
        message_window_push(
          "If available, history will be uploaded for viewing in the app config page daily during the noon sample.",
          false,
          false
        );
      }
    } break;
#endif

    // Other
    case MI_DELETE_ALL_DATA:
      if (s_reset_confirm) {
#ifdef FEATURE_SYNC
        comm_request_deletion();
#endif
        data_reset_all();
        vibes_double_pulse();
        window_stack_pop_all(true);
      } else {
        vibes_long_pulse();
      }

      s_reset_confirm = !s_reset_confirm;
      break;
    default: break;
  }

  menu_layer_reload_data(s_menu_layer);
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_header_layer = util_create_header_layer("Settings", 10);
  layer_add_child(root_layer, s_header_layer);

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(HEADER_INSET, 0, 0, 0)));
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
    .select_click = (MenuLayerSelectCallback)select_callback,
  });
  layer_add_child(root_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  layer_destroy(s_header_layer);

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

void settings_window_reload() {
  if(!s_window) return;
  
  menu_layer_reload_data(s_menu_layer);
}
