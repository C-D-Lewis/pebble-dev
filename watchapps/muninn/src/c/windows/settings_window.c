#include "settings_window.h"

static Window *s_window;
static Layer *s_canvas_layer;
static TextLayer *s_header_layer;
static MenuLayer *s_menu_layer;

static bool s_reset_confirm;

typedef enum {
  MI_VIBE_ON_SAMPLE,
  MI_CUSTOM_ALERT_LEVEL,
  MI_PUSH_TIMELINE_PINS,
  MI_ELEVATED_RATE_ALERT,
  MI_ONE_DAY_ALERT,

#ifdef FEATURE_SYNC
  MI_SYNC_INFO,
#endif
  MI_BATTERY_TIPS,
  MI_DELETE_ALL_DATA,
  MI_VERSION,

  MI_MAX,
} MenuItems;

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Title underline
  graphics_fill_rect(ctx, GRect(0, scl_y(110), PS_DISP_W, LINE_W), 0, GCornerNone);
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  PersistData *persist_data = data_get_persist_data();
  AppState *app_state = data_get_app_state();

  // Alert level detail
  const int alert_level = persist_data->custom_alert_level;
  const bool alert_disabled = alert_level == AL_OFF;
  static char s_alert_buff[24];
  if (!alert_disabled) {
    snprintf(s_alert_buff, sizeof(s_alert_buff), "Notify near %d%%", alert_level);
  }

  // Sync status
  static char s_sync_buff[20];
  const int sync_count = app_state->sync_count;
  if (sync_count == STATUS_EMPTY) {
    snprintf(s_sync_buff, sizeof(s_sync_buff), "Loading...");
  } else {
    // None yet, or first sync is in progress
    if (sync_count == 0) {
      snprintf(s_sync_buff, sizeof(s_sync_buff), "Syncing soon");
    } else {
      const int sync_perc = (sync_count * 100) / MAX_SYNC_ITEMS;
      snprintf(s_sync_buff, sizeof(s_sync_buff), "%d samples (%d%%)", sync_count, sync_perc);
    }
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
#ifdef FEATURE_SYNC
    case MI_SYNC_INFO:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Sync to Phone",
        s_sync_buff
      );
      break;
#endif
    case MI_BATTERY_TIPS:
      util_menu_cell_draw(ctx, cell_layer, "Battery tips", NULL);
      break;
    case MI_DELETE_ALL_DATA:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Delete All Data",
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
#ifdef FEATURE_SYNC
    case MI_SYNC_INFO:
#endif
    case MI_VERSION:
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

    // Other
#ifdef FEATURE_SYNC
    case MI_SYNC_INFO: {
      AppState *app_state = data_get_app_state();
      if (app_state->sync_count > 0) stats_window_push();
    } break;
#endif
    case MI_BATTERY_TIPS:
      message_window_push(MSG_TIPS, false, false);
      break;
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
    case MI_VERSION:
      message_window_push(MSG_INFORMATION, false, false);
      break;
    default: break;
  }

  menu_layer_reload_data(s_menu_layer);
}

static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_header_layer = util_make_text_layer(
    GRect(0, scl_y_pp({-30, .c = -20, .e = -25}), PS_DISP_W, 100),
    scl_get_font(SFI_Small)
  );
  text_layer_set_text(s_header_layer, "Settings");
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  layer_add_child(root_layer, text_layer_get_layer(s_header_layer));

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(HEADER_INSET, 0, 0, 0)));
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
    .select_click = (MenuLayerSelectCallback)select_callback,
  });
  layer_add_child(root_layer, menu_layer_get_layer(s_menu_layer));

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_header_layer);
  layer_destroy(s_canvas_layer);

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
