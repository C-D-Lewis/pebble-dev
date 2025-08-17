// A feat of UI engineering - why not BOTH designs?
#include "unified_window.h"

// Which screen of the unified design window?
typedef enum {
  StateToggles = 0,
  StateStatus
} ScrollState;

static Window *s_window;
static Layer *s_status_layer, *s_toggles_layer;
static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;

static GBitmap *s_up_bitmap, *s_down_bitmap;
static ScrollState s_scroll_state;
static int s_selection;
static bool s_interaction_enabled;
static int s_last_activated;

static bool waiting(int index) {
  return data_get_toggle_state(data_get_toggle_type_at_index(index)) == ToggleStateWaiting;
}

/********************************** Actions ***********************************/

static void action_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  const int type = data_get_toggle_type_at_index(s_last_activated);
  const int state_now = data_get_toggle_state(type);

  int new_state;
  if(type == ToggleTypeRinger) {
    // Get new state encoded in action (Thanks, Tyler!)
    new_state = *(int*)action_menu_item_get_action_data(action);
  } else {
    // Reverse/act on current state
    switch(type) {
      case ToggleTypeWifi: 
      case ToggleTypeData: 
      case ToggleTypeSync: 
      case ToggleTypeFindPhone: 
      case ToggleTypeWifiAP:
        // On/Off
        new_state = (state_now == ToggleStateOn) ? ToggleStateOff : ToggleStateOn;
        break;
      case ToggleTypeBluetooth: 
        // Off only
        new_state = ToggleStateOff;
        break;
      case ToggleTypeLockPhone: 
        // Lock only
        new_state = ToggleStateOn;  // ignored
        break;
      case ToggleTypeAutoBrightness: 
        // Auto or Manual TODO: Alias this to on (auto) or off (manual) and lump in above
        new_state = (state_now == ToggleStateBrightnessAutomatic) ? ToggleStateBrightnessManual : ToggleStateBrightnessAutomatic;
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown type for ActionMenu callback: %d", type);
        new_state = -1;
        break;
      }
  }

  if(new_state >= 0 && comm_check_bt("Disconnected. Reconnect phone to continue.")) {
    // Restrictions
    if(type == ToggleTypeData && data_get_toggle_state(ToggleTypeWifi) == ToggleStateOn) {
      dialog_window_push("Not available when Wi-Fi is on.");
      return;
    }

    // Disable while request is in flight
    data_set_toggle_state(type, ToggleStateWaiting);

    // Send request
    if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting type/state %d/%d", type, new_state);
    comm_request_toggle(type, new_state);
  }
}

static void handle_toggle() {
  if(waiting(s_last_activated)) {
    static char s_buff[64];
    char * fmt = PBL_IF_ROUND_ELSE("%s\nrequest already in progress.", "%s request already in progress.");
    snprintf(s_buff, sizeof(s_buff), fmt, list_window_get_type_string(data_get_toggle_type_at_index(s_last_activated)));
    dialog_window_push(&s_buff[0]);
    return;
  }

  const int type = data_get_toggle_type_at_index(s_selection);
  const int state = data_get_toggle_state(type);
  s_last_activated = s_selection;

  if(s_root_level) {
    action_menu_hierarchy_destroy(s_root_level, NULL, NULL);
    s_root_level = NULL;
  }

  // Prep action menu if required
  const int max_items = (type == ToggleTypeRinger) ? 2 : 1;
  if(max_items == 1) {
    // Single action, call action window callback directly
    action_callback(NULL, NULL, NULL);
  } else {
    // Show action menu
    s_root_level = action_menu_level_create(max_items);

    switch(type) {
      case ToggleTypeWifi: 
      case ToggleTypeData: 
      case ToggleTypeSync: 
      case ToggleTypeFindPhone: 
      case ToggleTypeWifiAP:
        // On/Off
        action_menu_level_add_action(s_root_level, (data_get_toggle_state(type) == ToggleStateOn) ? "Turn off" : "Turn on", action_callback, NULL);
        break;
      case ToggleTypeBluetooth: 
        // Off only
        action_menu_level_add_action(s_root_level, "Turn off", action_callback, NULL);
        break;
      case ToggleTypeRinger: 
        switch(state) {
          case ToggleStateLoud: {
            static int value = ToggleStateVibrate;
            action_menu_level_add_action(s_root_level, "Set to vibrate", action_callback, &value);
            static int value2 = ToggleStateSilent;
            action_menu_level_add_action(s_root_level, "Set to silent", action_callback, &value2);
          } break;
          case ToggleStateVibrate: {
            static int value = ToggleStateLoud;
            action_menu_level_add_action(s_root_level, "Set to loud", action_callback, &value);
            static int value2 = ToggleStateSilent;
            action_menu_level_add_action(s_root_level, "Set to silent", action_callback, &value2);
          } break;
          case ToggleStateSilent: {
            static int value = ToggleStateLoud;
            action_menu_level_add_action(s_root_level, "Set to loud", action_callback, &value);
            static int value2 = ToggleStateVibrate;
            action_menu_level_add_action(s_root_level, "Set to vibrate", action_callback, &value2);
          } break;
          default:
            APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown ringer state for ActionMenu: %d", state);
            break;
        }
        break;
      case ToggleTypeLockPhone: 
        action_menu_level_add_action(s_root_level, "Lock Phone", action_callback, NULL);
        break;
      case ToggleTypeAutoBrightness: 
        action_menu_level_add_action(s_root_level, (state == ToggleStateBrightnessAutomatic) ? "Set to manual" : "Set to automatic", action_callback, NULL);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown type for ActionMenu: %d", type);
        break;
    }

    ActionMenuConfig config = (ActionMenuConfig) {
      .root_level = s_root_level,
      .colors = {
        .background = PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorWhite),
        .foreground = GColorWhite
      },
      .align = ActionMenuAlignCenter
    };
    s_action_menu = action_menu_open(&config);
  }

  unified_window_reload();
}

/*********************************** State ************************************/

static bool interaction_enabled() {
  return s_interaction_enabled && !animation_is_locked();
}

static void state_end_callback(void *context) {
  animation_unlock();
  layer_mark_dirty(s_status_layer);
}

static void set_scroll_state(ScrollState state) {
  if(animation_is_locked()) {
    // Don't animate while animating
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted to change state while locked!");
    return;
  }

  if(state == s_scroll_state) {
    return;
  }
  s_scroll_state = state;

  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  switch(s_scroll_state) {
    case StateStatus:
      animation_animate_delta_y(s_status_layer, bounds.size.h, TRANSITION_DURATION, NULL);
      animation_animate_delta_y(s_toggles_layer, bounds.size.h, TRANSITION_DURATION, NULL);
      break;
    case StateToggles:
      animation_animate_delta_y(s_status_layer, -bounds.size.h, TRANSITION_DURATION, NULL);
      animation_animate_delta_y(s_toggles_layer, -bounds.size.h, TRANSITION_DURATION, NULL);
      break;
  }

  animation_lock();
  app_timer_register(TRANSITION_DURATION, state_end_callback, NULL);
}

/*********************************** Clicks ***********************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!interaction_enabled()) {
    return;
  }

  switch(s_scroll_state) {
    case StateStatus:
      list_window_push();
      return;
    case StateToggles:
      // Toggle the thing
      handle_toggle();
      break;
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) { 
  if(!interaction_enabled()) {
    return;
  }

  bool is_repeating = click_recognizer_is_repeating(recognizer);
  if(is_repeating && s_selection == 0) {
    // Don't repeat click up to status, its disorenting
    return;
  }
  
  switch(s_selection) {
    case 0:
      set_scroll_state(StateStatus);
      break;
    default:
      s_selection--;
      break;
  }

  layer_mark_dirty(s_toggles_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) { 
  if(!interaction_enabled()) {
    return;
  }

  switch(s_scroll_state) {
    case StateStatus:
      set_scroll_state(StateToggles);
      return;
    case StateToggles:
      break;
  } 
  
  if(s_selection != (ToggleTypeCount - 1)) {
    s_selection++;
  }

  layer_mark_dirty(s_toggles_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);

  const int interval_ms = 150;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, interval_ms, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, interval_ms, down_click_handler);
}

/********************************** Graphics **********************************/

static void status_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  // I love the graphics API!

  // Background
  const int x_offset = PBL_IF_ROUND_ELSE(20, 3);
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray));
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Title
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, 30), 0, GCornerNone);
  const GFont title_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  const int title_y = PBL_IF_ROUND_ELSE(-1, -3);
  GRect text_rect = GRect(0, title_y, bounds.size.w, 30);
  const char *title = PBL_IF_ROUND_ELSE("Status", "Phone Status");
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, title, title_font, text_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  // GSM
  const int margin = PBL_IF_ROUND_ELSE(x_offset + 7, 10);
  const int initial_y = PBL_IF_ROUND_ELSE(42, 40);
  const GSize icon_size = GSize(18, 18);
  const GSize box_size = GSize(bounds.size.w - (2 * margin), icon_size.h);
  const char *gsm_name = data_get_gsm_name();
  const int gsm_connected = strcmp(gsm_name, "Unknown") != 0;
  GRect field_rect = GRect(margin, initial_y, box_size.w, box_size.h);
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(gsm_connected ? GColorDarkCandyAppleRed : GColorDarkGray, GColorBlack));
  graphics_fill_rect(ctx, field_rect, 0, GCornerNone);

  // GSM - name
  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  const int text_x = 2;
  const int text_y = -4;
  text_rect = grect_inset(field_rect, GEdgeInsets(text_y, text_x, 0, icon_size.w));
  graphics_draw_text(ctx, gsm_name, font, text_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // GSM - level
  GRect bars[] = {
    GRect(margin + 4, field_rect.origin.y + 2 + 9, 3, 5),
    GRect(margin + 4 + 5, field_rect.origin.y + 2 + 5, 3, 9),
    GRect(margin + 4 + 10, field_rect.origin.y + 2, 3, 14)
  };
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(gsm_connected ? GColorDarkGray : GColorLightGray, GColorDarkGray));
  for(int i = 0; i < 3; i++) {
    graphics_fill_rect(ctx, bars[i], 0, GCornerNone);
  }
  if(gsm_connected) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    int perc = data_get_gsm_percent();
    if(perc >= 25 && perc < 33) {
      graphics_fill_rect(ctx, bars[0], 0, GCornerNone);
    } else if(perc >= 33 && perc < 66) {
      graphics_fill_rect(ctx, bars[0], 0, GCornerNone);
      graphics_fill_rect(ctx, bars[1], 0, GCornerNone);
    } else {
      for(int i = 0; i < 3; i++) {
        graphics_fill_rect(ctx, bars[i], 0, GCornerNone);
      }
    }
  }

  // Wifi
  const int gap = PBL_IF_ROUND_ELSE(11, 9);
  const bool wifi_on = data_get_toggle_state(ToggleTypeWifi) == ToggleStateOn;
  const char *wifi_name = wifi_on ? data_get_wifi_name() : "OFF";
  const bool wifi_connected = strcmp(wifi_name, "Disconnected") != 0 && wifi_on && strcmp(wifi_name, "Unknown") != 0;
  field_rect.origin.y += box_size.h + gap;
  text_rect = grect_inset(field_rect, GEdgeInsets(text_y, text_x, 0, icon_size.w));
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(wifi_connected ? GColorDarkCandyAppleRed : GColorDarkGray, GColorBlack));
  graphics_fill_rect(ctx, field_rect, 0, GCornerNone);
  graphics_draw_text(ctx, wifi_name, font, text_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Wifi - icon
  GPoint center = GPoint(field_rect.origin.x + (icon_size.w / 2) + 2, field_rect.origin.y + (icon_size.h / 2) + 4);
  int radius = 2;
  graphics_context_set_fill_color(ctx, wifi_connected ? GColorWhite : GColorLightGray);
  graphics_fill_circle(ctx, center, radius);
  graphics_fill_radial(ctx, GRect(center.x - gap, center.y - gap + PBL_IF_ROUND_ELSE(1, 0), (2 * gap) + 1, (2 * gap)), 
    GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(-50), DEG_TO_TRIGANGLE(50));
  graphics_fill_radial(ctx, GRect(center.x - gap + 2, center.y - gap + PBL_IF_ROUND_ELSE(1, 0) + 4, (2 * gap) + 1 - 4, (2 * gap) - 4), 
    GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(-45), DEG_TO_TRIGANGLE(45));

  // Battery
  field_rect.origin.y += box_size.h + gap;
  field_rect.size.w -= field_rect.size.w / 2;
  field_rect.size.w -= gap / 4;
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
  graphics_fill_rect(ctx, field_rect, 0, GCornerNone);
  text_rect = grect_inset(field_rect, GEdgeInsets(text_y, text_x, 0, icon_size.w));
  static char s_battery_buffer[8];
  int batt_perc = data_get_battery_percent();
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", batt_perc);
  graphics_draw_text(ctx, s_battery_buffer, font, text_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Battery - tip
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  const int battery_width = 16;
  GRect battery_box = GRect(field_rect.origin.x + 3, field_rect.origin.y + 4, battery_width, 10);
  graphics_draw_rect(ctx, battery_box);
  GRect tip_rect = GRect(battery_box.origin.x + battery_box.size.w, battery_box.origin.y + 2, 2, 6);
  graphics_fill_rect(ctx, tip_rect, 0, GCornerNone);

  // Battery - level
  const int width = (battery_width * batt_perc) / 100;
  battery_box.size.w = width;
  graphics_fill_rect(ctx, battery_box, 0, GCornerNone);

  // Storage
  field_rect.origin.x += field_rect.size.w;
  field_rect.origin.x += gap / 2;
  field_rect.origin.x--;
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
  graphics_fill_rect(ctx, field_rect, 0, GCornerNone);
  text_rect = grect_inset(field_rect, GEdgeInsets(text_y, text_x, 0, icon_size.w));
  static char s_storage_buffer[8];
  const int major = data_get_storage_gb_major();
  if(major >= 10) {
    snprintf(s_storage_buffer, sizeof(s_storage_buffer), "%dGB", major);
  } else {
    snprintf(s_storage_buffer, sizeof(s_storage_buffer), "%d.%dGB", major, data_get_storage_gb_minor());
  }
  graphics_draw_text(ctx, s_storage_buffer, font, text_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Storage - icon
  center = GPoint(field_rect.origin.x + (icon_size.w / 2), field_rect.origin.y + (icon_size.h / 2) - 1);
  radius = 6;
  int perc_full = data_get_storage_percent();
  int angle_full = (perc_full * 360) / 100;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_radial(ctx, GRect(center.x - radius, center.y - radius, (2 * radius) + 1, (2 * radius) + 1), 
    GOvalScaleModeFitCircle, 30, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(angle_full));
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_circle(ctx, center, radius);

  // Events option
  const int y_margin = PBL_IF_ROUND_ELSE(126, 119);
  GRect events_rect = PBL_IF_ROUND_ELSE(
    GRect(47, y_margin, 84, box_size.h + 10),
    GRect(30, y_margin, 83, box_size.h + 10)
  );
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
  graphics_fill_rect(ctx, events_rect, 0, GCornerNone);
  GColor fg_color = GColorWhite;
  graphics_context_set_stroke_color(ctx, fg_color);
  graphics_context_set_text_color(ctx, fg_color);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_rect(ctx, events_rect);
  events_rect = grect_inset(events_rect, GEdgeInsets(-3, 5, 0, 0));
  graphics_draw_text(ctx, "Events", title_font, events_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);

  // Events icon
  GRect icon_rect = GRect(events_rect.origin.x + 5, events_rect.origin.y + 8, 18, 18);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 3);
  center = grect_center_point(&icon_rect);
  radius = icon_rect.size.w / 2;
  graphics_draw_circle(ctx, center, radius);
  graphics_draw_line(ctx, center, GPoint(center.x, center.y - 5));
  graphics_draw_line(ctx, center, GPoint(center.x + 3, center.y));

  // Down arrow
  GSize down_size = gbitmap_get_bounds(s_down_bitmap).size;
  const int x_margin = (bounds.size.w - down_size.w) / 2;
  GRect down_rect = GRect(x_margin, PBL_IF_ROUND_ELSE(166, 158), down_size.w, down_size.h);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, down_rect.origin.y - 4, bounds.size.w, 20), 0, GCornerNone);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_down_bitmap, down_rect);

  // I now hate the graphics API...
}

static int xy2i(int x, int y, int row_size) {
  return (y * row_size) + x;
}

static GColor toggle_get_state_color(int index) {
  int type = data_get_toggle_type_at_index(index);
  int state = data_get_toggle_state(type);

  if(state == ToggleStateWaiting) {
    return GColorDarkGray;
  }

  switch(type) {
    // Always 'on'
    case ToggleTypeBluetooth:
    case ToggleTypeRinger:
      return GColorWhite;

    // On or Off
    case ToggleTypeWifi:
    case ToggleTypeData:
    case ToggleTypeSync:
    case ToggleTypeWifiAP:
    case ToggleTypeFindPhone:
      switch(state) {
        case ToggleStateOn:  return GColorWhite;
        case ToggleStateOff:
        default:             return GColorBlack;
      }

    // Special states
    case ToggleTypeAutoBrightness:
      return data_get_toggle_state(type) == ToggleStateBrightnessAutomatic ? GColorWhite : GColorBlack;
    case  ToggleTypeLockPhone:
      return GColorBlack;
    default: return GColorBlue;
  }
}

static void toggle_update_proc(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);

  // Down arrow
  if(s_selection == 0) {
    GSize up_size = gbitmap_get_bounds(s_up_bitmap).size;
    const int x_margin = (bounds.size.w - up_size.w) / 2;
    const int up_hint_height = PBL_IF_ROUND_ELSE(16, 14);
    GRect up_rect = GRect(x_margin, PBL_IF_ROUND_ELSE(5, 3), up_size.w, up_size.h);
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray));
    graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, up_hint_height), 0, GCornerNone);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_up_bitmap, up_rect);
  }

  // Toggle backgrounds
  const int radius = 5; 
  const int margin = 6;
  const GPoint corner_origin = GPoint(PBL_IF_ROUND_ELSE(28, 10), PBL_IF_ROUND_ELSE(27, 22));
  const GSize icon_size = GSize(37, 37);
  for(int y = 0; y < ToggleTypeCount / 3; y++) {
    for(int x = 0; x < ToggleTypeCount / 3; x++) {
      const int index = xy2i(x, y, 3);

      GRect outer = GRect(corner_origin.x + (x * margin) + (x * icon_size.w), 
                          corner_origin.y + (y * margin) + (y * icon_size.h), 
                          icon_size.w, icon_size.h);

      // Current selection indicator
      if(index == s_selection) {
        GRect larger = grect_inset(outer, GEdgeInsets(-3));
        graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorDarkGray));
        graphics_fill_rect(ctx, larger, radius, GCornersAll);
      }

      GColor state_color = toggle_get_state_color(index);
      graphics_context_set_fill_color(ctx, state_color);
      
      graphics_fill_rect(ctx, outer, radius, GCornersAll);
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, grect_inset(outer, GEdgeInsets(3)), radius, GCornersAll);

      // Icon
      GBitmap *bitmap = icons_get_with_type(data_get_toggle_type_at_index(index));
      GSize bitmap_size = gbitmap_get_bounds(bitmap).size;
      const int icon_x_margin = (outer.size.w - bitmap_size.w) / 2;
      const int icon_y_margin = (outer.size.h - bitmap_size.h) / 2;
      GRect icon_rect = GRect(outer.origin.x + icon_x_margin, outer.origin.y + icon_y_margin, bitmap_size.w, bitmap_size.h);
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, bitmap, icon_rect);
    }
  }

  // Current selection text
  const GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  const int type_height = PBL_IF_ROUND_ELSE(32, 23);
  const GRect type_rect = grect_inset(bounds, GEdgeInsets(bounds.size.h - type_height, 0, 0, 0));
  graphics_context_set_text_color(ctx, GColorWhite);
  const char *str = list_window_get_type_string(data_get_toggle_type_at_index(s_selection));
  graphics_draw_text(ctx, str, font, type_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_scroll_state = StateToggles;

  s_up_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UP);
  s_down_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DOWN);
  icons_load();

  s_toggles_layer = layer_create(bounds);
  layer_set_update_proc(s_toggles_layer, toggle_update_proc);
  layer_add_child(window_layer, s_toggles_layer);

  const int initial_y = -bounds.size.h;
  s_status_layer = layer_create(GRect(0, initial_y, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_status_layer, status_update_proc);
  layer_add_child(window_layer, s_status_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_status_layer);
  layer_destroy(s_toggles_layer);

  gbitmap_destroy(s_up_bitmap);
  gbitmap_destroy(s_down_bitmap);
  icons_unload();

  if(s_root_level) {
    action_menu_hierarchy_destroy(s_root_level, NULL, NULL);
    s_root_level = NULL;
  }

  window_destroy(s_window);
  s_window = NULL;
}

/************************************ API *************************************/

void unified_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap free: %d", (int)heap_bytes_free());
}

void unified_window_set_interaction_enabled(bool b) {
  s_interaction_enabled = b;
}

void unified_window_remove_from_stack() {
  if(s_window) {
    window_stack_remove(s_window, true);
  }
}

void unified_window_jump_to(int type) {
  s_selection = data_get_toggle_index_with_type(type);
  layer_mark_dirty(s_toggles_layer);
}

void unified_window_reload() {
  layer_mark_dirty(s_status_layer);
  layer_mark_dirty(s_toggles_layer);
}
