#include "settings_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static Layer *s_header_layer;

typedef enum {
  MI_SETTINGS,
#ifdef FEATURE_SYNC
  MI_SYNC_INFO,
  MI_UPLOAD,
#endif
  MI_BATTERY_TIPS,
  MI_VERSION,

  MI_MAX,
} MenuItems;

#ifdef FEATURE_SYNC
static char s_upload_buff[32];
#endif

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  PersistData *persist_data = data_get_persist_data();
  AppState *app_state = data_get_app_state();

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
      snprintf(s_sync_buff, sizeof(s_sync_buff), "%d samples", sync_count);
    }
  }

  switch(cell_index->row) {
    case MI_SETTINGS:
      util_menu_cell_draw(ctx, cell_layer, "Settings", NULL);
      break;
#ifdef FEATURE_SYNC
    case MI_SYNC_INFO:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "Historical Stats",
        s_sync_buff
      );
      break;
    case MI_UPLOAD:
      util_menu_cell_draw(
        ctx,
        cell_layer,
        "View All on Web",
        s_upload_buff
      );
      break;
#endif
    case MI_BATTERY_TIPS:
      util_menu_cell_draw(ctx, cell_layer, "Battery tips", NULL);
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
#ifdef FEATURE_SYNC
    case MI_SYNC_INFO:
    case MI_UPLOAD:
#endif
    case MI_VERSION:
      return ROW_HEIGHT_LARGE;
    default:
      return ROW_HEIGHT_SMALL;
  }
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case MI_SETTINGS:
      settings_window_push();
      break;
#ifdef FEATURE_SYNC
    case MI_SYNC_INFO: {
      AppState *app_state = data_get_app_state();
      if (app_state->sync_count > 0) stats_window_push();
    } break;
    case MI_UPLOAD: {
      if (data_get_log_length() < MIN_SAMPLES_FOR_GRAPH) return;

      snprintf(s_upload_buff, sizeof(s_upload_buff), "Uploading...");
      comm_upload_history();
    } break;
#endif
    case MI_BATTERY_TIPS:
      message_window_push(MSG_TIPS, false, false);
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

#ifdef FEATURE_SYNC
  if (data_get_log_length() < MIN_SAMPLES_FOR_GRAPH) {
    snprintf(s_upload_buff, sizeof(s_upload_buff), "Not enough samples");
  } else {
    snprintf(s_upload_buff, sizeof(s_upload_buff), "Press to share");
  }
#endif

  window_stack_push(s_window, true);
}

void menu_window_reload() {
  if(!s_window) return;
  
  menu_layer_reload_data(s_menu_layer);
}

#ifdef FEATURE_SYNC
void menu_window_set_upload_status(const char *status) {
  snprintf(s_upload_buff, sizeof(s_upload_buff), "%s", status);
  menu_layer_reload_data(s_menu_layer);
}
#endif