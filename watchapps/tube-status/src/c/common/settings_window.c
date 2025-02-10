#include "settings_window.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static ServerStatus s_server_status = ServerStatusWaiting;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return SettingsWindowRowMax;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case SettingsWindowRowSubscription:
      menu_cell_basic_draw(ctx, cell_layer, "Delay Pins", settings_get_subscription_string(), NULL);
      break;
    case SettingsWindowRowServerStatus:
      if(!bluetooth_connection_service_peek()) {
        menu_cell_basic_draw(ctx, cell_layer, "Pin Server", "N/A (disconnected)", NULL);
        return;
      }
      
      switch(s_server_status) {
        case ServerStatusUp:
          menu_cell_basic_draw(ctx, cell_layer, "Pin Server", "Server is up", NULL);
          break;
        case ServerStatusWaiting:
          menu_cell_basic_draw(ctx, cell_layer, "Pin Server", "Querying status...", NULL);
          break;
        case ServerStatusTimeout:
          menu_cell_basic_draw(ctx, cell_layer, "Pin Server", "Server may be down :(", NULL);
          break;
      }
      break;
    case SettingsWindowRowAbout: {
      static char s_version_buff[16];
      snprintf(s_version_buff, sizeof(s_version_buff), "Version %s", VERSION);
      menu_cell_basic_draw(ctx, cell_layer, s_version_buff, "Powered by tfl.gov.uk", NULL);
    } break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
        MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    44);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case SettingsWindowRowSubscription: {
        SubscriptionState state = settings_get_subscription_state();
        state++;
        if(state == SubscriptionStateMax) {
          state = SubscriptionStateUnsubscribed;
        }
        settings_set_subscription_state(state);
      }
      break;
    default: break;
  }

  menu_layer_reload_data(s_menu_layer);
}

static int16_t get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return STATUS_BAR_LAYER_HEIGHT;
}

static void draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *context) {
  menu_cell_basic_header_draw(ctx, cell_layer, "Press Back to apply");
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorDukeBlue, GColorWhite);
#endif
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
    .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
    .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
    .select_click = (MenuLayerSelectCallback)select_callback,
#if !defined(PBL_ROUND)
    .get_header_height = (MenuLayerGetHeaderHeightCallback)get_header_height_callback,
    .draw_header = (MenuLayerDrawHeaderCallback)draw_header_callback,
#endif
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

}

static void main_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  // Self destroying
  window_destroy(window);
  s_main_window = NULL;

  // Resync with splash
  splash_window_push();
}

static void main_window_appear(Window *window) {
  // Refresh that row
  s_server_status = ServerStatusWaiting;
  comm_get_server_status();
}

void settings_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .appear  = main_window_appear,
        .load = main_window_load,
        .unload = main_window_unload
    });
  }
  window_stack_push(s_main_window, true);
}

void settings_window_update_server_status(ServerStatus s) {
  s_server_status = s;
  menu_layer_reload_data(s_menu_layer);
}
