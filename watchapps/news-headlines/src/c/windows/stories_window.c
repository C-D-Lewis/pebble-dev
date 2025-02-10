#include "stories_window.h"

static Window *s_window;
static Layer *s_title_layer;
static GBitmap *s_wrench_bitmap, *s_thumb_icon_bitmap;
static TextLayer *s_status_layer;
static ContentIndicator *s_indicator;
static Layer *s_up_indicator_layer, *s_down_indicator_layer;

static int s_fake_para_widths[3];
static int s_index = 0;
static bool s_is_animating;

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  s_is_animating = false;
}

static void animate_in(int offset) {
  if(!s_is_animating) {
    s_is_animating = true;

    GRect finish = layer_get_frame(s_title_layer);
    GRect start = layer_get_frame(s_title_layer);
    start.origin.y += offset;

    PropertyAnimation *prop_anim = property_animation_create_layer_frame(s_title_layer, &start, &finish);
    Animation *anim = property_animation_get_animation(prop_anim);
    animation_set_handlers(anim, (AnimationHandlers) {
      .stopped = anim_stopped_handler
    }, NULL);
    animation_set_duration(anim, 150);
    animation_schedule(anim);
  }
}

static void regenerate_fake_paragraph_widths() {
  s_fake_para_widths[0] = (rand() % 40) + 10;
  s_fake_para_widths[1] = (rand() % 40) + 10;
  s_fake_para_widths[2] = (rand() % 40) + 10;
}

static void title_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  Story *story = data_get_story(s_index);

  // Background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Top sep
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(bounds.origin.x + 3, bounds.origin.y + 3, bounds.size.w - 6, 2), 0, GCornerNone);

  // Logo
  // const int h_width = 100;
  // const GEdgeInsets logo_insets = {
  //   .top = 6,
  //   .left = (bounds.size.w - h_width) / 2,
  //   .bottom = bounds.size.h - 8,
  //   .right = (bounds.size.w - h_width) / 2
  // };
  // GRect logo_bounds = grect_inset(bounds, logo_insets);

  // Thumbnail icon hint - not since image sizes changed in June 2016 :(
// #if defined(PBL_COLOR)
//   if(story->has_image) {
//     const int hint_size = 15; // bitmap size
//     GRect thumb_icon_rect = GRect(logo_bounds.origin.x + 100, logo_bounds.origin.y - 1, hint_size, hint_size);
//     graphics_draw_bitmap_in_rect(ctx, s_thumb_icon_bitmap, thumb_icon_rect);
//   }
// #endif

  // Bottom sep
  graphics_fill_rect(ctx, GRect(bounds.origin.x + 3, bounds.origin.y + 7, bounds.size.w - 6, 3), 0, GCornerNone);

  // Title
  GRect title_bounds = grect_inset(bounds, GEdgeInsets(5, 3, 0, 3));
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, story->title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    title_bounds, GTextOverflowModeWordWrap,
    GTextAlignmentCenter, NULL);

  GSize title_size = graphics_text_layout_get_content_size(story->title,
    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), title_bounds,
    GTextOverflowModeWordWrap, GTextAlignmentCenter);

  // Dummy text boxes
  graphics_context_set_fill_color(ctx, GColorLightGray);
  srand(time(NULL));
  int y = bounds.origin.y + 3 + title_size.h + 10;
  graphics_fill_rect(ctx, GRect(
    bounds.origin.x + 5, y,
    bounds.size.w - s_fake_para_widths[0], (s_fake_para_widths[0] / 2)), 0, GCornerNone);
  y += (s_fake_para_widths[0] / 2) + 3;
  graphics_fill_rect(ctx, GRect(
    bounds.origin.x + 5, y,
    bounds.size.w - s_fake_para_widths[1], (s_fake_para_widths[1] / 2)), 0, GCornerNone);
  y += (s_fake_para_widths[1] / 2) + 3;
  int last_height = (s_fake_para_widths[2] / 2);
  graphics_fill_rect(ctx, GRect(
    bounds.origin.x + 5, y,
    bounds.size.w - s_fake_para_widths[2], 168), 0, GCornerNone); // Fill the rest
}

static void update_data() {
  if(s_title_layer) {
    // Show updated data for current story
    Story *story = data_get_story(s_index);

    static char s_status_buffer[32];
    snprintf(s_status_buffer, sizeof(s_status_buffer), "%d/%d", s_index + 1, data_get_quantity());
    text_layer_set_text(s_status_layer, s_status_buffer);

    content_indicator_set_content_available(s_indicator, ContentIndicatorDirectionUp, (s_index > 0));
    content_indicator_set_content_available(s_indicator, ContentIndicatorDirectionDown, (s_index < data_get_quantity() - 1));

    layer_mark_dirty(s_title_layer);
  }
}

/*********************************** Clicks ***********************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!s_is_animating) {
    detail_window_push(s_index);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!s_is_animating) {
    if(s_index > 0) {
      s_index--;
      update_data();
      animate_in(STORIES_WINDOW_ANIM_OFFSET);
      regenerate_fake_paragraph_widths();
    } else {
      if(click_recognizer_is_repeating(recognizer)) {
        return;
      }
      
      if(connection_service_peek_pebble_app_connection()) {
        // Show settings
        settings_window_push();
      } else {
        // End of cached data
        animate_in(STORIES_WINDOW_ANIM_OFFSET / 2);
        vibes_double_pulse();
      }
    }
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if(!s_is_animating) {
    if(s_index < data_get_quantity() - 1) {
      s_index++;
      update_data();
      animate_in(STORIES_WINDOW_ANIM_OFFSET);
      regenerate_fake_paragraph_widths();
    } else if(!click_recognizer_is_repeating(recognizer)) {
      // End reached
      animate_in(STORIES_WINDOW_ANIM_OFFSET / 2);
      vibes_double_pulse();
    }
  }
}

static void click_config_provider(void *context) {
  const int interval_ms = 100;
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT, interval_ms, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, interval_ms, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, interval_ms, down_click_handler);
}

/*********************************** Window ***********************************/

static void up_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // If not in cache mode
  if(connection_service_peek_pebble_app_connection()) {
    // Show wrench when not a ContentIndicator
    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    GRect bitmap_bounds = gbitmap_get_bounds(s_wrench_bitmap);
    const int x_margin = (bounds.size.w - bitmap_bounds.size.w) / 2;
    const int y_margin = PBL_IF_ROUND_ELSE(3, 3);
    graphics_draw_bitmap_in_rect(ctx, s_wrench_bitmap, GRect(x_margin, y_margin, bitmap_bounds.size.w, bitmap_bounds.size.h));
  }
}

static void window_load(Window *this) {
  Layer *window_layer = window_get_root_layer(this);
  GRect bounds = layer_get_bounds(window_layer);

  s_wrench_bitmap = gbitmap_create_with_resource(RESOURCE_ID_GEAR);
  s_thumb_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUMB_ICON);

  const int margin = (bounds.size.w - 144) / 2;
  const GEdgeInsets title_insets = {
    .top = STATUS_BAR_LAYER_HEIGHT + PBL_IF_ROUND_ELSE(20, 3),
    .left = margin,
    .right = margin
  };
  s_title_layer = layer_create(grect_inset(bounds, title_insets));
  layer_set_update_proc(s_title_layer, title_update_proc);
  layer_add_child(window_layer, s_title_layer);

  const int indicator_margin = PBL_IF_ROUND_ELSE(STATUS_BAR_LAYER_HEIGHT, STATUS_BAR_LAYER_HEIGHT - 3);

  s_up_indicator_layer = layer_create(grect_inset(bounds, GEdgeInsets(0, 0, bounds.size.h - indicator_margin - PBL_IF_ROUND_ELSE(0, 5), 0)));
  layer_set_update_proc(s_up_indicator_layer, up_update_proc);
  layer_add_child(window_layer, s_up_indicator_layer);
  s_down_indicator_layer = layer_create(grect_inset(bounds, GEdgeInsets(bounds.size.h - indicator_margin, 0, 0, 0)));
  layer_add_child(window_layer, s_down_indicator_layer);

  s_indicator = content_indicator_create();
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionUp, &(ContentIndicatorConfig) {
    .layer = s_up_indicator_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorWhite,
      .background = GColorDarkCandyAppleRed
    }
  });
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionDown, &(ContentIndicatorConfig) {
    .layer = s_down_indicator_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorBlack,
      .background = GColorClear
    }
  });

  #if defined(PBL_ROUND)
  GRect status_bounds = GRect(0, indicator_margin - 8, bounds.size.w, 36);
#elif defined(PBL_RECT)
  GRect status_bounds = GRect(0, -3, bounds.size.w - 5, 36);
#endif
  s_status_layer = text_layer_create(status_bounds);
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_alignment(s_status_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  text_layer_set_overflow_mode(s_status_layer, GTextOverflowModeTrailingEllipsis);
  layer_add_child(window_layer, text_layer_get_layer(s_status_layer));

  update_data();
  regenerate_fake_paragraph_widths();
}

static void window_unload(Window *this) {
  layer_destroy(s_title_layer);
  gbitmap_destroy(s_wrench_bitmap);
  gbitmap_destroy(s_thumb_icon_bitmap);
  text_layer_destroy(s_status_layer);
  content_indicator_destroy(s_indicator);
  layer_destroy(s_up_indicator_layer);
  layer_destroy(s_down_indicator_layer);

  // Self destroy
  window_destroy(s_window);
  s_window = NULL;

  // Don't show splash on the way out
  window_stack_pop_all(true);
}

/************************************ API *************************************/

void stories_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorBlack));
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }
  window_stack_push(s_window, true);

  s_index = 0;
  update_data();
}
