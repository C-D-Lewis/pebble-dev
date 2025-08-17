#include "list_window.h"

#define BUFFER_LENGTH 32
#define ALL_EVENTS    -1

static Window *s_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

static Event events[MAX_WAKEUPS];
static char list_buffers[MAX_WAKEUPS][BUFFER_LENGTH];

char* list_window_get_type_string(int type) {
  switch(type) {
    case ToggleTypeWifi:            return "Wi-Fi";
    case ToggleTypeData:            return "Data";
    case ToggleTypeBluetooth:       return "Bluetooth";
    case ToggleTypeRinger:          return "Ringer";
    case ToggleTypeSync:            return "Auto Sync";
    case ToggleTypeWifiAP:          return "Hotspot";
    case ToggleTypeFindPhone:       return "Find Phone";
    case ToggleTypeAutoBrightness:  return "Auto Bright.";
    case ToggleTypeLockPhone:       return "Lock Phone";  
    default: 
      APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown type for list_window_get_type_string: %d", type);
      return "Unknown";
  }
}

static void reload_events(int specific) {
  if(specific != ALL_EVENTS) {
    // Reload just one
    if(persist_exists(specific)) {
      persist_read_data(specific, &events[specific], sizeof(Event));
    } else {
      events[specific].set = 0; // Not valid
    }
  } else {
    // Reload them all
    for(int i = 0; i < MAX_WAKEUPS; i++) {
      // If this row's event exists...
      if(persist_exists(i)) {
        // Load it
        persist_read_data(i, &events[i], sizeof(Event));

        if(DEBUG) {
          // Log it
          time_t timestamp;
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Read index %d, %d: %s (%d:%d): valid: %s",
            i, events[i].type, wakeup_window_get_state_string(events[i].state), events[i].hour,
            events[i].minute, (wakeup_query(events[i].wakeup_id, &timestamp) ? "true" : "false")
          );
        }
      } else {
        // Does not yet exist
        events[i].set = 0;
        if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Event %d is not set or does not exist", i);
      }
    }
  }
}

static int get_total_occupied_rows() {
  int result = 0;
  for(int i = 0; i < MAX_WAKEUPS; i++) {
    result += events[i].set;  // 1 or 0, neat
  }
  return result;
}

/*********************************** MenuLayer ********************************/

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  GRect bounds = layer_get_bounds(cell_layer);
  int row = cell_index->row;

  if(events[row].set == 0) {
    // Not yet set, unused
    if(menu_layer_is_index_selected(s_menu_layer, cell_index)) {
      menu_cell_basic_draw(ctx, cell_layer, "Unused", "Click to add event...",  NULL);\
    } else {
      menu_cell_basic_draw(ctx, cell_layer, "Unused", NULL,  NULL);\
    }
  } else {
    // Used, load the data
    int type = events[row].type;
    int state = events[row].state;
    int hour = events[row].hour;
    int minute = events[row].minute;

    if(menu_layer_is_index_selected(s_menu_layer, cell_index)) {
      // Show all the details (TODO: padding function)
      if(hour < 10 && minute < 10) {
        snprintf(list_buffers[row], BUFFER_LENGTH, "%s: %s\n(0%d:0%d)", list_window_get_type_string(type),
            wakeup_window_get_state_string(state), hour, minute);
      } else if(hour < 10 && minute > 9) {
        snprintf(list_buffers[row], BUFFER_LENGTH, "%s: %s\n(0%d:%d)", list_window_get_type_string(type),
            wakeup_window_get_state_string(state), hour, minute);
      } else if(hour > 9 && minute < 10) {
        snprintf(list_buffers[row], BUFFER_LENGTH, "%s: %s\n(%d:0%d)", list_window_get_type_string(type),
            wakeup_window_get_state_string(state), hour, minute);
      } else {
        snprintf(list_buffers[row], BUFFER_LENGTH, "%s: %s\n(%d:%d)", list_window_get_type_string(type),
            wakeup_window_get_state_string(state), hour, minute);
      }
    } else {
      // Show minimal detail
      snprintf(list_buffers[row], BUFFER_LENGTH, "%s: %s", list_window_get_type_string(type),
            wakeup_window_get_state_string(state));
    }

    int y_margin = 0;
    GRect text_rect = grect_inset(bounds, GEdgeInsets(y_margin, 0, 0, PBL_IF_ROUND_ELSE(0, 5)));
    graphics_draw_text(ctx, list_buffers[row], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), text_rect, 
      GTextOverflowModeWordWrap, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
    text_rect.origin.y += 37;
    graphics_draw_text(ctx, "Click to delete...", fonts_get_system_font(FONT_KEY_GOTHIC_18), text_rect, 
      GTextOverflowModeWordWrap, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft), NULL);
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return menu_layer_is_index_selected(menu_layer, cell_index) ?
    (3 * MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT) / 4 : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT;
}

static uint16_t num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MAX_WAKEUPS;  // TODO revisit collapsible list structure
}

static void select_click_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  int row = cell_index->row;

  // Event for this row?
  if(persist_read_data(row, &events[row], sizeof(Event)) <= 0) {
    // Does not exist, add it with user input
    schedule_window_push(row);
  } else {
    // Exists, remove wakeup and delete data
    wakeup_cancel(events[row].wakeup_id);
    persist_delete(row);
    dialog_window_push("Event deleted.");
  }

  // If none remain
  if(get_total_occupied_rows() == 0) {
    // Cancel All Wakeups
    wakeup_cancel_all();
    if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Cancelling all wakeups...");

    for(int i = 0; i < MAX_WAKEUPS; i++) {
      if(persist_read_data(i, &events[i], sizeof(Event)) > 0) {
        // Save and reload only this event
        persist_delete(i);
      }
    }
  }

  // Update view
  reload_events(ALL_EVENTS);
  menu_layer_reload_data(menu_layer);
}

static uint16_t get_num_sections_callback(struct MenuLayer *menu_layer, void *context) {
  const int num_sections = 1;
  return num_sections;
}

/************************************ Window **********************************/

static void window_appear(Window *window) {
  // Load once all events
  reload_events(ALL_EVENTS);
  menu_layer_reload_data(s_menu_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(PBL_IF_ROUND_ELSE(0, STATUS_BAR_LAYER_HEIGHT), 0, 0, 0)));
  menu_layer_set_center_focused(s_menu_layer, true);
  menu_layer_pad_bottom_enable(s_menu_layer, false);
#if defined(PBL_COLOR)
  menu_layer_set_normal_colors(s_menu_layer, GColorBlack, GColorWhite);
  menu_layer_set_highlight_colors(s_menu_layer, GColorDarkCandyAppleRed, GColorWhite);
#endif
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .draw_row = draw_row_callback,
    .get_cell_height = get_cell_height_callback,
    .get_num_rows = num_rows_callback,
    .select_click = select_click_callback,
    .get_num_sections = get_num_sections_callback
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_separator_mode(s_status_layer, StatusBarLayerSeparatorModeDotted);
  status_bar_layer_set_colors(s_status_layer, 
    PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorWhite), PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  status_bar_layer_destroy(s_status_layer);

  window_destroy(window);
  s_window = NULL;
}

void list_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear
    });
  }
  window_stack_push(s_window, true);
}
