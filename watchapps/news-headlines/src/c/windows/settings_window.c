#include "settings_window.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static ServerStatus s_server_status = ServerStatusWaiting;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return SETTINGS_WINDOW_NUM_SETTINGS;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case SettingsTypeRegion:
      menu_cell_basic_draw(ctx, cell_layer, "News Region", settings_get_region_string(), NULL);
      break;
    case SettingsTypeCategory:
      if(settings_get_region() == RegionUK) {
        menu_cell_basic_draw(ctx, cell_layer, "News Category", settings_get_category_string(), NULL);
      } else {
        menu_cell_basic_draw(ctx, cell_layer, "News Category", "(UK Region Only)", NULL);
      }
      break;
    case SettingsTypePins:
      menu_cell_basic_draw(ctx, cell_layer, "UK Headlines Pins", settings_get_subscribed_string(), NULL);
      break;
    case SettingsTypeNumStories:
      menu_cell_basic_draw(ctx, cell_layer, "Number of Stories", settings_get_num_stories_string(), NULL);
      break;
    case SettingsTypeFontSize:
      menu_cell_basic_draw(ctx, cell_layer, "Font Size", settings_get_font_size_string(), NULL);
      break;
    case SettingsTypeStatus:
      if(bluetooth_connection_service_peek()) {
        switch(s_server_status) {
          case ServerStatusUp:
            menu_cell_basic_draw(ctx, cell_layer, "Server Status", "Server is up", NULL);
            break;
          case ServerStatusWaiting:
            menu_cell_basic_draw(ctx, cell_layer, "Server Status", "Querying status...", NULL);
            break;
          case ServerStatusDown:
          case ServerStatusTimeout:
          default:
            menu_cell_basic_draw(ctx, cell_layer, "Server Status", "Server may be down :(", NULL);
            break;
        }
      } else {
        menu_cell_basic_draw(ctx, cell_layer, "Pin Server Status", "Phone disconnected", NULL);
      }
      break;
    case SettingsTypeAbout: {
      static char s_version_buff[16];
      snprintf(s_version_buff, sizeof(s_version_buff), "Version %s", VERSION);
      menu_cell_basic_draw(ctx, cell_layer, s_version_buff, "Powered by BBC News", NULL);
    } break;
    case SettingsTypeDonate:
      menu_cell_basic_draw(ctx, cell_layer, "Donations", "paypal.me/chrislewis1", NULL);
      break;
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
    case SettingsTypeRegion: {
      int num = settings_get_region();
      num += (num < RegionCount - 1) ? 1 : -(RegionCount - 1);
      settings_set_region(num);
    } break;
    case SettingsTypeCategory: {
      if(settings_get_region() == RegionUK) {
        int num = settings_get_category();
        num += (num < CategoryCount - 1) ? 1 : -(CategoryCount - 1);
        settings_set_category(num);
      }
    } break;
    case SettingsTypePins: {
      PinSubscriptionType type = settings_get_subscribed_type();
      if(type == PinSubscriptionTypeNotSubscribed) {
        settings_set_subscribed_type(PinSubscriptionTypeSubscribed);
      } else {
        settings_set_subscribed_type(PinSubscriptionTypeNotSubscribed);
      }
    } break;
    case SettingsTypeNumStories:
      switch(settings_get_number_of_stories()) {
        case 10:
          settings_set_number_of_stories(20);
          break;
        case 20:
          settings_set_number_of_stories(10);
          break;
      }
      break;
    case SettingsTypeFontSize:
      switch(settings_get_font_size()) {
        case FontSizeSmall:
          settings_set_font_size(FontSizeLarge);
          break;
        case FontSizeLarge:
          settings_set_font_size(FontSizeSmall);
          break;
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
  menu_layer_set_highlight_colors(s_menu_layer, GColorDarkCandyAppleRed, GColorWhite);
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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap free: %d", (int)heap_bytes_free());
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
  comm_get_pin_server_status();
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

void settings_window_update_pin_server_status(ServerStatus s) {
  s_server_status = s;
  menu_layer_reload_data(s_menu_layer);
}
