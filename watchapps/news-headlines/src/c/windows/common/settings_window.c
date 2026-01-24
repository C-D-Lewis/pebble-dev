#include "settings_window.h"

#define NUM_SETTINGS 4
#define ROW_HEIGHT_LARGE scalable_y(300)

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return NUM_SETTINGS;
}

// Like menu_cell_basic_draw but with larger subtitle
static void menu_cell_draw(GContext *ctx, Layer *layer, char *title, char *desc) {
  PreferredContentSize content_size = preferred_content_size();
  if (content_size <= PreferredContentSizeMedium) {
    menu_cell_basic_draw(ctx, layer, title, desc, NULL);
    return;
  }

#if defined(PBL_PLATFORM_CHALK)
  // Use Chalk's text flowing
  menu_cell_basic_draw(ctx, layer, title, desc, NULL);
  return;
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
  switch(cell_index->row) {
    case SettingsTypeRegion:
      menu_cell_draw(ctx, cell_layer, "News Region", settings_get_region_string());
      break;
    case SettingsTypeCategory:
      if (settings_get_region() == RegionUK) {
        menu_cell_draw(ctx, cell_layer, "News Category", settings_get_category_string());
      } else {
        menu_cell_draw(ctx, cell_layer, "News Category", "(UK Region Only)");
      }
      break;
    case SettingsTypeNumStories:
      menu_cell_draw(ctx, cell_layer, "Number of Stories", settings_get_num_stories_string());
      break;
    case SettingsTypeAbout:
      menu_cell_draw(ctx, cell_layer, "News Headlines", "Powered by BBC News");
      break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index)
      ? MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT
      : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    ROW_HEIGHT_LARGE
  );
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  switch(cell_index->row) {
    case SettingsTypeRegion: {
      int num = settings_get_region();
      num += (num < RegionCount - 1) ? 1 : -(RegionCount - 1);
      settings_set_region(num);
    } break;
    case SettingsTypeCategory: {
      if (settings_get_region() == RegionUK) {
        int num = settings_get_category();
        num += (num < CategoryCount - 1) ? 1 : -(CategoryCount - 1);
        settings_set_category(num);
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
    default: break;
  }

  menu_layer_reload_data(s_menu_layer);
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

void settings_window_push() {
  if (!s_main_window) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
  }
  window_stack_push(s_main_window, true);
}
