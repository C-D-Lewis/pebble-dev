#include "menu_window.h"

#define ROW_HEIGHT_SMALL scalable_y(220)
#define ROW_HEIGHT_LARGE scalable_y(300)
#define MIN_SAMPLES_FOR_GRAPH 4

static Window *s_window;
static MenuLayer *s_menu_layer;

static bool s_reset_confirm;

typedef enum {
  MI_VIBE_ON_SAMPLE = 0,
  MI_CUSTOM_ALERT_LEVEL,
  MI_PUSH_TIMELINE_PINS,
  MI_ELEVATED_RATE_ALERT,
  MI_GRAPH,
  MI_BATTERY_TIPS,
  MI_DELETE_ALL_DATA,
  MI_VERSION,
  MI_MAX,
} MenuItems;

static bool graph_is_available() {
  return data_get_log_length() >= MIN_SAMPLES_FOR_GRAPH;
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return MI_MAX;
}

// Like menu_cell_basic_draw but with larger subtitle
static void menu_cell_draw(GContext *ctx, Layer *layer, char *title, char *desc) {
  // TODO: Can we use ContentSize here without layout issues?
  //       It may conflict with pebble-scalable font system
  PreferredContentSize content_size = preferred_content_size();
  // APP_LOG(APP_LOG_LEVEL_INFO, "content_size: %d", (int)content_size);

#if !defined(TEST_FORCE_SCALING)
  // Medium or smaller (rare?), use regular rendering
  if (content_size <= PreferredContentSizeMedium) {
    menu_cell_basic_draw(ctx, layer, title, desc, NULL);
    return;
  }
#endif

  // Else, use larger one
  GRect title_rect = scalable_grect_pp(
    GRect(30, -30, 1000, 300),
    GRect(30, -10, 1000, 300)
  );
  if (desc == NULL) {
    title_rect.origin.y += scalable_y(30);
  }

  graphics_draw_text(
    ctx,
    title,
    scalable_get_font(SFI_MediumBold),
    title_rect,
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  if (desc != NULL) {
    graphics_draw_text(
      ctx,
      desc,
      scalable_get_font(SFI_Medium),
      scalable_grect_pp(
        GRect(30, 110, 1000, 300),
        GRect(30, 130, 1000, 300)
      ),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft,
      NULL
    );
  }
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  // Alert level detail
  const int alert_level = data_get_custom_alert_level();
  const bool alert_disabled = alert_level == AL_OFF;
  static char s_alert_buff[24];
  if (!alert_disabled) {
    snprintf(s_alert_buff, sizeof(s_alert_buff), "Notifying near %d%%", alert_level);
  }

  // Graph availability after 4 samples
  const bool graph_valid = graph_is_available();
  static char s_graph_buff[22];
  snprintf(s_graph_buff, sizeof(s_graph_buff), "Need min. %d samples", MIN_SAMPLES_FOR_GRAPH);

  switch(cell_index->row) {
    case MI_VIBE_ON_SAMPLE:
      menu_cell_draw(
        ctx,
        cell_layer,
        "Vibrate on sample",
        data_get_vibe_on_sample() ? "Enabled" : "Disabled"
      );
      break;
    case MI_CUSTOM_ALERT_LEVEL:
      menu_cell_draw(
        ctx,
        cell_layer,
        "Custom alert",
        alert_disabled ? "Disabled" : s_alert_buff
      );
      break;
    case MI_PUSH_TIMELINE_PINS:
      menu_cell_draw(
        ctx,
        cell_layer,
        "Timeline pins",
        data_get_push_timeline_pins() ? "Enabled" : "Disabled"
      );
      break;
    case MI_ELEVATED_RATE_ALERT:
      menu_cell_draw(
        ctx,
        cell_layer,
        "High drain alert",
        data_get_elevated_rate_alert() ? "Enabled" : "Disabled"
      );
      break;
    case MI_GRAPH:
      menu_cell_draw(
        ctx,
        cell_layer,
        "Log graph",
        graph_valid ? NULL : s_graph_buff
      );
      break;
    case MI_BATTERY_TIPS:
      menu_cell_draw(ctx, cell_layer, "Battery tips", NULL);
      break;
    case MI_DELETE_ALL_DATA:
      menu_cell_draw(
        ctx,
        cell_layer,
        "Delete all data",
        s_reset_confirm ? "Tap again to confirm" : NULL
      );
      break;
    case MI_VERSION: {
      static char s_v_buff[16];
      snprintf(s_v_buff, sizeof(s_v_buff), "Version %s", VERSION);
      menu_cell_draw(
        ctx,
        cell_layer,
        "Muninn",
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
    case MI_VERSION:
      return ROW_HEIGHT_LARGE;
    case MI_GRAPH:
      return graph_is_available() ? ROW_HEIGHT_SMALL : ROW_HEIGHT_LARGE;
    case MI_DELETE_ALL_DATA:
      return s_reset_confirm ? ROW_HEIGHT_LARGE : ROW_HEIGHT_SMALL;
    default:
      return ROW_HEIGHT_SMALL;
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
    case MI_PUSH_TIMELINE_PINS: {
      const bool now_enabled = !data_get_push_timeline_pins();
      data_set_push_timeline_pins(now_enabled);

      if (now_enabled) comm_push_timeline_pins();
    } break;
    case MI_ELEVATED_RATE_ALERT:
      data_set_elevated_rate_alert(!data_get_elevated_rate_alert());
      break;
    case MI_GRAPH: {
      if (graph_is_available()) {
        graph_window_push();
      } else {
        vibes_long_pulse();
      }
    } break;
    case MI_BATTERY_TIPS:
      message_window_push(MSG_TIPS);
      break;
    case MI_DELETE_ALL_DATA:
      if (s_reset_confirm) {
        data_reset_all();
        vibes_double_pulse();
        window_stack_pop_all(true);
      } else {
        vibes_long_pulse();
      }

      s_reset_confirm = !s_reset_confirm;
    case MI_VERSION:
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

  s_reset_confirm = false;

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
