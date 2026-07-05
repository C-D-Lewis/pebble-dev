#include "main_window.h"

#define HEADER_HEIGHT scl_y(550)
#define HEADER_HINT_HEIGHT scl_y(100)

static Window *s_window;
static Layer *s_header_layer, *s_toggles_layer;

static int s_selection = 0;
static bool s_is_animating = false, s_header_visible = false;

/******************************************** Drawing *********************************************/

static void header_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();

  graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);

  // Device name
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    app_state->device_name,
    scl_get_font(SFI_Medium),
    GRect(scl_x(10), scl_y(10), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Battery percentage
  static char battery_str[8];
  snprintf(battery_str, sizeof(battery_str), "%s%%", app_state->battery_perc);
  graphics_draw_text(
    ctx,
    battery_str,
    scl_get_font(SFI_Medium),
    GRect(scl_x(10), scl_y(120), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  // Free space (perc)
  static char free_space_str[32];
  snprintf(free_space_str, sizeof(free_space_str), "%s (%s%% free)", app_state->free_space, app_state->free_space_perc);
  graphics_draw_text(
    ctx,
    free_space_str,
    scl_get_font(SFI_Medium),
    GRect(scl_x(10), scl_y(230), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
}

static void toggles_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();
  
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    app_state->toggle_order,
    scl_get_font(SFI_Medium),
    layer_get_bounds(layer),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
}

/******************************************* Animations *******************************************/

static void animation_started_handler(Animation *anim, void *context) {
  s_is_animating = true;
}

static void animation_stopped_handler(Animation *anim, bool finished, void *context) {
  s_is_animating = false;
}

void animate_layer(Layer *layer, GRect start, GRect finish, int duration_ms) {
  if (s_is_animating) return;

  PropertyAnimation *prop_anim = property_animation_create_layer_frame(layer, &start, &finish);
  Animation *anim = property_animation_get_animation(prop_anim);
  animation_set_duration(anim, duration_ms);
  animation_set_curve(anim, AnimationCurveEaseOut);
  animation_set_handlers((Animation*)anim, (AnimationHandlers) {
    .started = animation_started_handler,
    .stopped = animation_stopped_handler
  }, NULL);
  animation_schedule(anim);
}

void animate_delta_y(Layer *layer, int dy, int duration_ms) {
  GRect start = layer_get_frame(layer);
  GRect finish = GRect(start.origin.x, start.origin.y + dy, start.size.w, start.size.h);
  animate_layer(layer, start, finish, duration_ms);
}

/********************************************* Clicks *********************************************/

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // if 0, go up, else decrement
  if (s_selection > 0) {
    s_selection--;
    return;
  }

  if (!s_header_visible) {
    // Show header
    s_header_visible = true;
    animate_delta_y(s_header_layer, (HEADER_HEIGHT - HEADER_HINT_HEIGHT), 200);
  } else {
    // Hide header
    s_header_visible = false;
    animate_delta_y(s_header_layer, -(HEADER_HEIGHT - HEADER_HINT_HEIGHT), 200);
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // toggle selected
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_selection < TogglesMax) {
    s_selection++;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

/********************************************* Window *********************************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_toggles_layer = layer_create(grect_inset(bounds, GEdgeInsets(HEADER_HINT_HEIGHT, 0, 0, 0)));
  layer_set_update_proc(s_toggles_layer, toggles_update_proc);
  layer_add_child(window_layer, s_toggles_layer);

  // Initial position
  GRect header_bounds = GRect(
    0,
    -(HEADER_HEIGHT - HEADER_HINT_HEIGHT),
    bounds.size.w,
    HEADER_HEIGHT
  );
  s_header_layer = layer_create(header_bounds);
  layer_set_update_proc(s_header_layer, header_update_proc);
  layer_add_child(window_layer, s_header_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_header_layer);
  layer_destroy(s_toggles_layer);
  
  window_destroy(window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);

  comm_sync_data();
}

void main_window_update() {
  layer_mark_dirty(s_header_layer);
  layer_mark_dirty(s_toggles_layer);
}
