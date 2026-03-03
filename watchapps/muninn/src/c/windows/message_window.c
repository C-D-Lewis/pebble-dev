#include "message_window.h"

#if defined(PBL_PLATFORM_EMERY)
  #define BRAID_H 18
#else
  #define BRAID_H 14
#endif

#if defined(PBL_PLATFORM_APLITE)
  #define BRAID_Y 0
#else
  #define BRAID_Y scl_y(150)
#endif

static Window *s_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_text_layer;
static Layer *s_canvas_layer;

static char *s_text_ptr;

static void braid_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

#if !defined(PBL_PLATFORM_APLITE)
  const bool is_night = util_get_is_night();
  util_draw_skyline(ctx, is_night);

  graphics_draw_bitmap_in_rect(
    ctx,
    bitmaps_get(util_get_mascot_res_id(true, is_night)),
    GRect(
      scl_x_pp({.o = 400, .c = 430, .e = 400}),
      scl_y_pp({.o = 20, .c = 25, .e = 25}),
      MASCOT_SIZE,
      MASCOT_SIZE
    )
  );
#endif

  GRect braid_rect = GRect(0, BRAID_Y, bounds.size.w, BRAID_H);
  util_draw_braid(ctx, braid_rect);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  s_canvas_layer = layer_create(GRect(0, 0, PS_DISP_W, bounds.size.h));
  layer_set_update_proc(s_canvas_layer, braid_update_proc);
  layer_add_child(root_layer, s_canvas_layer);

  // Code from devsite to try and fit text inside a TextLayer inside a ScrollLayer
  const int margin = PBL_IF_ROUND_ELSE(8, 4);
  GRect shrinking_rect = GRect(margin, 0, bounds.size.w - (2 * margin), 4000);
  GSize text_size = graphics_text_layout_get_content_size(
    s_text_ptr,
    scl_get_font(SFI_Medium), 
    shrinking_rect,
    GTextOverflowModeWordWrap,
    GTextAlignmentLeft
  );
  text_size.h += 50;
  GRect text_bounds = bounds;
  text_bounds.origin.x += margin;
  text_bounds.size.w -= (2 * margin);
  text_bounds.size.h = text_size.h;

  s_text_layer = util_make_text_layer(text_bounds, scl_get_font(SFI_Medium));
  text_layer_set_text_alignment(
    s_text_layer,
    PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft)
  );
  text_layer_set_text(s_text_layer, s_text_ptr);
  
  const int scroll_y = BRAID_Y + BRAID_H;
  s_scroll_layer = scroll_layer_create(
    GRect(0, scroll_y, bounds.size.w, bounds.size.h - scroll_y)
  );
  scroll_layer_set_content_size(s_scroll_layer, text_size);
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(root_layer, scroll_layer_get_layer(s_scroll_layer));
}

static void window_unload(Window *window) {
  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(s_text_layer);
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

static void timer_callback(void *data) {
  window_stack_pop_all(true);
}

void message_window_push(char *text, bool do_vibe, bool do_dismiss) {
  s_text_ptr = text;

  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }

  window_stack_push(s_window, true);

  if (do_dismiss) app_timer_register(3000, timer_callback, NULL);
  if (do_vibe && !quiet_time_is_active()) vibes_short_pulse();
}
