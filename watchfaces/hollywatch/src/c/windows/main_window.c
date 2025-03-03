#include "main_window.h"

#define WIDTH PBL_IF_ROUND_ELSE(180, 144)
#define HEIGHT PBL_IF_ROUND_ELSE(180, 168)
#define IMAGE_SIZE 140

#define HOLLY_Y_MARGIN PBL_IF_ROUND_ELSE(20, -10)
#define GRECT_HOLLY GRect((WIDTH - IMAGE_SIZE) / 2, ((HEIGHT + HOLLY_Y_MARGIN) - IMAGE_SIZE) / 2, IMAGE_SIZE, IMAGE_SIZE)  
#define GRECT_QUOTE GRect(0, HEIGHT - PBL_IF_ROUND_ELSE(35, 30), PBL_IF_ROUND_ELSE(WIDTH - 30, WIDTH), 30)
#define GRECT_TIME GRect(0, PBL_IF_ROUND_ELSE(20, 0), WIDTH, 30)

static Window *s_window;
static BitmapLayer *s_holly_layer;
static GBitmap *s_holly_bitmap;
static TextLayer *s_time_layer, *s_quote_layer;

static const char *s_quotes[] = {
  "There's an emergency going on",
  "Why are you asking me?",
  "Probably not serious, don't panic",
  "That's a poor IQ for a glass of water!",
  "Oh yeah, didn't see that",
  "You want me to prove it, do you?",
  "How 'bout a space question instead?",
  "I may not be fast, but I get there in the end",
  "Still, you've got to laugh, haven't you?",
  "My IQ is 6000 - same as 6000 PE teachers",
  "It's still going on",
  "Abandon shop! This is not a daffodil.",
  "Everybody's dead, Dave",
  "Gordon Bennett!",
  "They're dead, Dave.",
};

static int s_last_quote = -1;

/**
 * Handler when a tick occurs.
 */
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  static char time_buffer[32];
  strftime(time_buffer, sizeof(time_buffer), "Alright dudes? It's %H:%M", tick_time);
  text_layer_set_text(s_time_layer, time_buffer);

#if defined(PBL_RECT)
  // Random quote
  int quote = rand() % ARRAY_LENGTH(s_quotes);
  while(quote == s_last_quote) {
    quote = rand() % ARRAY_LENGTH(s_quotes);
  }
  s_last_quote = quote;
  text_layer_set_text(s_quote_layer, s_quotes[quote]);
#endif
}

/**
 * Window load event handler.
 */
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_holly_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HOLLY);

  s_holly_layer = bitmap_layer_create(GRECT_HOLLY);
  bitmap_layer_set_bitmap(s_holly_layer, s_holly_bitmap);
  bitmap_layer_set_compositing_mode(s_holly_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_holly_layer));

  s_time_layer = text_layer_create(GRECT_TIME);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

#if defined(PBL_RECT)
  s_quote_layer = text_layer_create(GRect(0, bounds.size.h - 30, bounds.size.w, 30));
  text_layer_set_text_alignment(s_quote_layer, GTextAlignmentCenter);
  text_layer_set_font(s_quote_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_quote_layer, GColorWhite);
  text_layer_set_background_color(s_quote_layer, GColorClear);
  text_layer_set_overflow_mode(s_quote_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_quote_layer));
#endif
}

/**
 * Window unload event handler.
 */
static void window_unload(Window *window) {
  bitmap_layer_destroy(s_holly_layer);
  text_layer_destroy(s_time_layer);
#if defined(PBL_RECT)
  text_layer_destroy(s_quote_layer);
#endif

  gbitmap_destroy(s_holly_bitmap);

  window_destroy(s_window);
  s_window = NULL;
}

/**
 * Load the main window and push it to the stack.
 */
void main_window_push() {
  if(!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_background_color(s_window, GColorBlack);
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  time_t now = time(NULL);
  struct tm *tick_now = localtime(&now);
  tick_handler(tick_now, MINUTE_UNIT);
}
