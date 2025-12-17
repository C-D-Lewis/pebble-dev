#include "splash_window.h"

#define BAR_WIDTH scalable_x(450)

static Window *s_window;
static BitmapLayer *s_logo_layer;
static Layer *s_bar_layer;

static GBitmap *s_logo_bitmap;
static AppTimer *s_timeout_timer;
static bool s_first_view = true;
static int s_quantity = 0, s_progress = 0;

static void timeout_handler(void *context) {
  s_timeout_timer = NULL;

  comm_set_fast(true);
  comm_send_settings();
  s_timeout_timer = app_timer_register(COMM_TIMEOUT_MS, timeout_handler, NULL);
}

void splash_window_begin() {
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  comm_send_settings();
  s_timeout_timer = app_timer_register(COMM_TIMEOUT_MS, timeout_handler, NULL);
}

static void load_cache_handler(void *context) {
  splash_window_cancel_timeout();
  if (data_load_cached_data()) {
    comm_set_fast(false);
#if defined(PBL_ROUND)
    stories_window_round_push();
#else
    stories_window_rect_push();
#endif
  } else {
    // Cache not available (unlikely since even the first launch will be after install, which implies connection)
    splash_window_cancel_timeout();
  }
}

/**
 * Simple progress bar with inset
 */
static void progress_bar_update_proc(Layer *layer, GContext *ctx) {
  const int cap_radius = scalable_x(35);
  const int inset = 2;

  // Background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(cap_radius, cap_radius), cap_radius);
  graphics_fill_rect(
    ctx,
    GRect(cap_radius, 0, BAR_WIDTH - (2 * cap_radius), (2 * cap_radius) + inset - 1),
    0,
    GCornerNone
  );
  graphics_fill_circle(ctx, GPoint(BAR_WIDTH - cap_radius, cap_radius), cap_radius);

  // Bar progress - subtract one to avoid overrun
  if (s_progress < 1) return;
  
  const int width = ((s_quantity > 0 ? s_progress - 1 : s_progress) * BAR_WIDTH) / s_quantity;
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(cap_radius, cap_radius), cap_radius - inset);
  graphics_fill_rect(
    ctx,
    GRect(cap_radius + inset, inset, width - (2 * inset) + 1, (2 * cap_radius) - inset - 1),
    0,
    GCornerNone
  );
  graphics_fill_circle(ctx, GPoint(cap_radius + width, cap_radius), cap_radius - inset);
}

static void window_load(Window *this) {
  Layer *window_layer = window_get_root_layer(this);
  GRect bounds = layer_get_bounds(window_layer);

  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_APP_ICON_SMALL);
  GSize logo_size = gbitmap_get_bounds(s_logo_bitmap).size;

  const int logo_x_margin = (bounds.size.w - logo_size.w) / 2;
  const int logo_y_margin = (bounds.size.h - logo_size.h) / 2;
  s_logo_layer = bitmap_layer_create(GRect(logo_x_margin, logo_y_margin, logo_size.w, logo_size.h));
  bitmap_layer_set_compositing_mode(s_logo_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_layer));

  const int x_margin = (bounds.size.w - BAR_WIDTH) / 2;
  s_bar_layer = layer_create(
    GRect(
      x_margin,
      logo_y_margin + logo_size.h + scalable_y(100),
      bounds.size.w - (2 * x_margin),
      25
    )
  );
  layer_set_update_proc(s_bar_layer, progress_bar_update_proc);
  layer_add_child(window_layer, s_bar_layer);
  layer_set_hidden(s_bar_layer, true);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(s_logo_layer);
  layer_destroy(s_bar_layer);
  gbitmap_destroy(s_logo_bitmap);

  window_destroy(window);
  s_window = NULL;

  window_stack_pop_all(true);
}

static void window_appear(Window *window) {
  if (bluetooth_connection_service_peek()) {
    // Cater for returning when applying
    if (s_first_view) {
      s_first_view = false;
    } else {
      // 'ready' won't come mid-app, go anyway
      splash_window_begin();
    }
  } else {
    if (persist_exists(DATA_PERSIST_KEY_CACHED)) {
      app_timer_register(1500, load_cache_handler, NULL);
    } else {
      // Can't load from cache, no data available
      splash_window_cancel_timeout();
    }
  }
}

void splash_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorLightGray));
    window_set_window_handlers(s_window, (WindowHandlers) {
      .appear = window_appear,  // Special case to reset layout state
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  s_quantity = data_get_quantity();
  s_progress = 0;
}

static void next_window_handler() {
  splash_window_cancel_timeout();
#if defined(PBL_ROUND)
    stories_window_round_push();
#else
    stories_window_rect_push();
#endif
}

void splash_window_set_progress(int progress) {
  layer_set_hidden(s_bar_layer, false);
  s_quantity = data_get_quantity();
  s_progress = progress;

  // Update progress bar
  layer_mark_dirty(s_bar_layer);

  if (progress == s_quantity - 1) {
    // All here
    data_cache_data();
    comm_set_fast(false);
    next_window_handler();
  }
}

void splash_window_cancel_timeout() {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer);
    s_timeout_timer = NULL;
  }
}
