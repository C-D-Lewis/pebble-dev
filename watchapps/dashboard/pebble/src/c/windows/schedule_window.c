#include "schedule_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

static int s_chosen_hour = 0, s_chosen_minute = 0;
static int s_chosen_type = ToggleTypeWifi;
static int s_chosen_state = ToggleStateOff;
static int s_list_position;
static bool s_invalid = false;

/**
 * Get the next WeekDay based on today, as API cannot schedule every day
 */
WeekDay schedule_window_get_tomorrow_weekday() {
  time_t temp = time(NULL);
  struct tm *now = localtime(&temp);
  switch(now->tm_wday) {
    case 0:  return MONDAY;
    case 1:  return TUESDAY;
    case 2:  return WEDNESDAY;
    case 3:  return THURSDAY;
    case 4:  return FRIDAY;
    case 5:  return SATURDAY;
    case 6:  return SUNDAY;
    default: return TODAY;
  }
}

/************************************* MenuLayer ******************************/

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    // Which toggle?
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, list_window_get_type_string(s_chosen_type), "Choose toggle", NULL);
      break;

    // On or off?
    case 1:
      if(s_chosen_type == ToggleTypeBluetooth) {
        menu_cell_basic_draw(ctx, cell_layer, "OFF ONLY", "Choose state", NULL);
      } else if(s_chosen_type == ToggleTypeRinger) {
        switch(s_chosen_state) {
          case ToggleStateLoud:
            menu_cell_basic_draw(ctx, cell_layer, "LOUD", "Choose state", NULL);
            break;
          case ToggleStateVibrate:
            menu_cell_basic_draw(ctx, cell_layer, "VIBRATE", "Choose state", NULL);
            break;
          case ToggleStateSilent:
            menu_cell_basic_draw(ctx, cell_layer, data_get_is_lollipop() ? "PRIORITY" : "SILENT", "Choose state", NULL);
            break;
          default:
            menu_cell_basic_draw(ctx, cell_layer, "UNKNOWN", "Choose state", NULL);
            break;
        }
      } else if(s_chosen_type == ToggleTypeAutoBrightness) {
        menu_cell_basic_draw(ctx, cell_layer, 
          (s_chosen_state == ToggleStateBrightnessAutomatic) ? "AUTO" : "MANUAL", "Choose state", NULL);
      } else {
        menu_cell_basic_draw(ctx, cell_layer, (s_chosen_state == ToggleStateOn) ? "ON" : "OFF", "Choose state", NULL);
      }
      break;

    // What time?
    case 2: {
      static char time_buff[16];
      if(s_chosen_hour < 10 && s_chosen_minute < 10) {
        snprintf(time_buff, sizeof(time_buff), "0%d:0%d", s_chosen_hour, s_chosen_minute);
      } else if(s_chosen_hour < 10 && s_chosen_minute > 9) {
        snprintf(time_buff, sizeof(time_buff), "0%d:%d", s_chosen_hour, s_chosen_minute);
      } else if(s_chosen_hour > 9 && s_chosen_minute < 10) {
        snprintf(time_buff, sizeof(time_buff), "%d:0%d", s_chosen_hour, s_chosen_minute);
      } else {
        snprintf(time_buff, sizeof(time_buff), "%d:%d", s_chosen_hour, s_chosen_minute);
      }
      menu_cell_basic_draw(ctx, cell_layer, time_buff, "Choose time", NULL);
    } break;

    // Submit
    case 3:
      menu_cell_basic_draw(ctx, cell_layer, s_invalid ? "Error registering!" : "Schedule Event", NULL, NULL);
      break;
    default:
      menu_cell_basic_draw(ctx, cell_layer, "UNKNOWN", NULL, NULL);
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    44);
}

static uint16_t num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *context) {
  const int num_rows = 4;
  return num_rows;
}

static void minute_selected_callback(int accepted_value) {
  s_chosen_minute = accepted_value;
}

static void hour_selected_callback(int accepted_value) {
  s_chosen_hour = accepted_value;

  // Choose minute
  new_number_window_push("Minute", 0, 55, 5, minute_selected_callback);
}

static void invalid_handler(void *context) {
  s_invalid = false;
  menu_layer_reload_data(s_menu_layer);
}

static void select_click_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  int row = cell_index->row;

  switch(row) {
    // Toggle selection window crashed, toggle manually
    case 0:
      s_chosen_type++;

      // Some are unavailable, skip them
      switch(s_chosen_type) {
        case ToggleTypeFindPhone:
          s_chosen_type += 2;
          break;
        case ToggleTypeLockPhone:
          s_chosen_type++;
          break;
        default:
          // Loop back to start
          if(s_chosen_type == ToggleTypeCount) {
            s_chosen_type = 0;
          }
          break;
      }

      // If set to ringer, need to check if it is not a valid ringer state
      if(s_chosen_type == ToggleTypeRinger) {
        if(s_chosen_state < ToggleStateLoud || s_chosen_state > ToggleStateSilent) {
          s_chosen_state = ToggleStateLoud;
        }

      // Check in right range here too
      } else if(s_chosen_type == ToggleTypeAutoBrightness) {
        if(s_chosen_state < ToggleStateBrightnessAutomatic || s_chosen_state > ToggleStateBrightnessManual) {
          s_chosen_state = ToggleStateBrightnessAutomatic;
        }

      // Only on or off
      } else {
        if(s_chosen_state > ToggleStateOn) {
          s_chosen_state = ToggleStateOff;
        }
      }
      break;

    // On/Off/Ringer type selection
    case 1:
      if(s_chosen_type == ToggleTypeBluetooth) {
        s_chosen_state = ToggleStateOff;
      } else if(s_chosen_type == ToggleTypeRinger) {
        switch(s_chosen_state) {
          case ToggleStateLoud:
            s_chosen_state = ToggleStateSilent;
            break;
          case ToggleStateVibrate:
            s_chosen_state = ToggleStateLoud;
            break;
          case ToggleStateSilent:
            s_chosen_state = ToggleStateVibrate;
            break;
        }
      } else if(s_chosen_type == ToggleTypeAutoBrightness) {
        if(s_chosen_state == ToggleStateBrightnessAutomatic) {
          s_chosen_state = ToggleStateBrightnessManual;
        } else {
          s_chosen_state = ToggleStateBrightnessAutomatic;
        }
      } else {
        // Toggle ON/OFF
        if(s_chosen_state == ToggleStateOff) {
          s_chosen_state = ToggleStateOn;
        } else {
          s_chosen_state = ToggleStateOff;
        }
      }
      break;

    // Time selection window sequence
    case 2:
      new_number_window_push("Hour (24h)", 0, 23, 1, hour_selected_callback);
      break;

    // Submit options, calculate timestamps, schedule wakeup, register with Persist
    case 3: {
      // Compare with now
      time_t temp = time(NULL);
      struct tm* tm = localtime(&temp);

      time_t timestamp = time(NULL); 
      if(s_chosen_hour > tm->tm_hour && s_chosen_minute > tm->tm_min) {  // Special formula
        // Tomorrow
        timestamp = clock_to_timestamp(schedule_window_get_tomorrow_weekday(), s_chosen_hour, s_chosen_minute);
      } else {
        // Later today
        timestamp = clock_to_timestamp(TODAY, s_chosen_hour, s_chosen_minute);
      }

      int id = wakeup_schedule(timestamp, s_list_position, false);
      if(id > 0) {
        if(!wakeup_query(id, &timestamp)) {
          // Get the time remaining
          int seconds_remaining = timestamp - time(NULL);
          if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Seconds remaining: %d", seconds_remaining);
        } else {
          if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Wakeup ID recall failed, got %d", id);
        }

        // Gate to ensure ON/OFF ONLY is adhered to if not clicked on above
        if(s_chosen_type == ToggleTypeBluetooth) {
          s_chosen_state = ToggleStateOff;
        } // Else leave as user wants

        // Store event data to Persist
        Event e = {
          .type = s_chosen_type,
          .state = s_chosen_state,
          .hour = s_chosen_hour,
          .minute = s_chosen_minute,
          .set = 1,
          .wakeup_id = id
        };
        persist_write_data(s_list_position, &e, sizeof(Event));

        // Pop back
        window_stack_pop(true);
        dialog_window_push("Event created.");
      } else {
        // Show error to user
        s_invalid = true;
        app_timer_register(3000, invalid_handler, NULL);
        menu_layer_reload_data(menu_layer);
      }
      return;
    } break;
  }

  menu_layer_reload_data(menu_layer);
}

/************************************ Window **********************************/

static void window_appear(Window *window) {
  menu_layer_reload_data(s_menu_layer);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(grect_inset(bounds, GEdgeInsets(PBL_IF_ROUND_ELSE(0, STATUS_BAR_LAYER_HEIGHT), 0, 0, 0)));
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
    .select_click = select_click_callback
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

void schedule_window_push(int position) {
  // Which slot in the calling list view window?
  s_list_position = position;

  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
    window_set_window_handlers(s_window, (WindowHandlers) {
      .appear = window_appear,
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);
}
