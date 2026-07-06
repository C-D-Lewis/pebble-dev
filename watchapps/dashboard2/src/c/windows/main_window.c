#include "main_window.h"

#define TRAY_HEIGHT scl_y(750)
#define TRAY_HINT_HEIGHT scl_y(100)

static Window *s_window;
static Layer
  *s_tray_layer,
  *s_toggles_layer;

static int s_selection = 0;
static bool
  s_out_of_date = false,
  s_synced = false,
  s_is_animating = false,
  s_tray_visible = false;

void animate_delta_y(Layer *layer, int dy, int duration_ms);

static void toggle_tray() {
  if (s_is_animating) return;

  // Arrow changes
  bitmaps_destroy_id(RESOURCE_ID_ARROW_DOWN);
  bitmaps_destroy_id(RESOURCE_ID_ARROW_UP);

  if (!s_tray_visible) {
    // Show tray
    s_tray_visible = true;
    animate_delta_y(s_tray_layer, (TRAY_HEIGHT - TRAY_HINT_HEIGHT), 200);
  } else {
    // Hide tray
    s_tray_visible = false;
    animate_delta_y(s_tray_layer, -(TRAY_HEIGHT - TRAY_HINT_HEIGHT), 200);
  }
}

/******************************************** Drawing *********************************************/

static void tray_update_proc(Layer *layer, GContext *ctx) {
  if (!s_synced || s_out_of_date) return;

  AppState *app_state = data_get_app_state();

  graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);

  const int x_margin = scl_x(160);

  // Device name
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    "Device",
    scl_get_font(SFI_Small),
    GRect(x_margin, scl_y(-20), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_text(
    ctx,
    app_state->device_name,
    scl_get_font(SFI_Medium),
    GRect(x_margin, scl_y(65), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_PHONE),
    GRect(scl_x(5), scl_y(50), 28, 28)
  );

  // Battery percentage
  graphics_draw_text(
    ctx,
    "Battery",
    scl_get_font(SFI_Small),
    GRect(x_margin, scl_y(190), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  static char battery_str[16];
  snprintf(battery_str, sizeof(battery_str), "%s%% left", app_state->battery_perc);
  graphics_draw_text(
    ctx,
    battery_str,
    scl_get_font(SFI_Medium),
    GRect(x_margin, scl_y(275), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_BATTERY),
    GRect(scl_x(5), scl_y(260), 28, 28)
  );

  // Free space (perc)
  graphics_draw_text(
    ctx,
    "Free Storage",
    scl_get_font(SFI_Small),
    GRect(x_margin, scl_y(400), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  static char free_space_str[32];
  snprintf(free_space_str, sizeof(free_space_str), "%s (%s%%)", app_state->free_space, app_state->free_space_perc);
  graphics_draw_text(
    ctx,
    free_space_str,
    scl_get_font(SFI_Medium),
    GRect(x_margin, scl_y(485), PS_DISP_W, 100),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(RESOURCE_ID_DISK),
    GRect(scl_x(5), scl_y(470), 28, 28)
  );

  // Dashed white separator line
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  for (int x = 0; x < PS_DISP_W; x += 8) {
    graphics_draw_line(ctx, GPoint(x, scl_y(640)), GPoint(x + 4, scl_y(640)));
  }

  // Arrow
  graphics_draw_bitmap_in_rect(
    ctx,
    s_tray_visible ? bitmaps_get(RESOURCE_ID_ARROW_UP) : bitmaps_get(RESOURCE_ID_ARROW_DOWN),
    GRect((PS_DISP_W - 28) / 2, scl_y(670), 24, 12)
  );
}

static void toggles_update_proc(Layer *layer, GContext *ctx) {
  AppState *app_state = data_get_app_state();
  GRect bounds = layer_get_bounds(layer);

  // Splash screen conditions
  if (!s_synced || s_out_of_date) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_APP_ICON),
      GRect((bounds.size.w - 50) / 2, ((bounds.size.h - 50) / 2) - (TRAY_HINT_HEIGHT / 2), 50, 50)
    );

    if (s_out_of_date) {
      static char s_err_buff[32];
      snprintf(
        s_err_buff,
        sizeof(s_err_buff),
        "Out of date\n(watch %d, mobile %d)",
        COMPAT_PROTOCOL_VERSION,
        app_state->compat_protocol_version
      );
      graphics_context_set_text_color(ctx, GColorWhite);
      graphics_draw_text(
        ctx,
        s_err_buff,
        scl_get_font(SFI_Medium),
        GRect(0, scl_y(500), bounds.size.w, 100),
        GTextOverflowModeTrailingEllipsis,
        GTextAlignmentCenter,
        NULL
      );
    }
    return;
  }

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
  if (!s_synced) return;
  if (s_selection > 0) {
    s_selection--;
    return;
  }

  toggle_tray();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!s_synced) return;

  // toggle selected
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (!s_synced) return;
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

  s_toggles_layer = layer_create(grect_inset(bounds, GEdgeInsets(TRAY_HINT_HEIGHT, 0, 0, 0)));
  layer_set_update_proc(s_toggles_layer, toggles_update_proc);
  layer_add_child(window_layer, s_toggles_layer);

  // Initial position
  GRect tray_bounds = GRect(
    0,
    -(TRAY_HEIGHT - TRAY_HINT_HEIGHT),
    bounds.size.w,
    TRAY_HEIGHT
  );
  s_tray_layer = layer_create(tray_bounds);
  layer_set_update_proc(s_tray_layer, tray_update_proc);
  layer_add_child(window_layer, s_tray_layer);

#ifdef TEST_EXPAND_TRAY
  toggle_tray();
#endif
}

static void window_unload(Window *window) {
  layer_destroy(s_tray_layer);
  layer_destroy(s_toggles_layer);

  window_destroy(window);
  s_window = NULL;
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_background_color(s_window, GColorDarkCandyAppleRed);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);

  comm_sync_data();
}

void main_window_update() {
  AppState *app_state = data_get_app_state();

  // Watchapp is out of date
  if (app_state->compat_protocol_version != COMPAT_PROTOCOL_VERSION) {
    s_out_of_date = true;
  } else {
    // OK!
    s_synced = true;

    window_set_background_color(s_window, GColorBlack);
  }

  layer_mark_dirty(s_tray_layer);
  layer_mark_dirty(s_toggles_layer);
}
