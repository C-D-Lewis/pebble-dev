#include "pge_splash.h"

static Window *s_splash_window = NULL;
static TextLayer *s_built_layer, *s_line_layer, *s_block_layer, *s_p_layer, *s_g_layer, *s_e_layer;

static PGESplashDoneHandler *s_done_handler;

static bool appeared = false;

static void animate(Layer *layer, GRect start, GRect finish, int duration, int delay) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  animation_set_duration((Animation*)anim, duration);
  animation_set_delay((Animation*)anim, delay);
  animation_set_curve((Animation*)anim, AnimationCurveEaseOut);
  animation_schedule((Animation*)anim);
}

static void next_callback(void *context) {
  s_done_handler();
}

static void hide_callback(void *context) {
  // Hide P, G, E
  GRect start = layer_get_frame(text_layer_get_layer(s_p_layer));
  GRect finish = GRect(start.origin.x, start.origin.y + 43, start.size.w, start.size.h);
  animate(text_layer_get_layer(s_p_layer), start, finish, 200, 0);

  start = layer_get_frame(text_layer_get_layer(s_g_layer));
  finish = GRect(start.origin.x, start.origin.y + 43, start.size.w, start.size.h);
  animate(text_layer_get_layer(s_g_layer), start, finish, 200, 100);

  start = layer_get_frame(text_layer_get_layer(s_e_layer));
  finish = GRect(start.origin.x, start.origin.y + 43, start.size.w, start.size.h);
  animate(text_layer_get_layer(s_e_layer), start, finish, 200, 200);

  // Hide the line
  start = layer_get_frame(text_layer_get_layer(s_line_layer));
  finish = GRect(144, start.origin.y, 0, start.size.h);
  animate(text_layer_get_layer(s_line_layer), start, finish, 300, 200);

  app_timer_register(1000, next_callback, NULL);
}

static void start_animations() {
  // Expand the line
  GRect start = layer_get_frame(text_layer_get_layer(s_line_layer));
  GRect finish = GRect(start.origin.x, start.origin.y, 144, start.size.h);
  animate(text_layer_get_layer(s_line_layer), start, finish, 500, 100);
  
  // Schedule P, G, E
  start = layer_get_frame(text_layer_get_layer(s_p_layer));
  finish = GRect(start.origin.x, start.origin.y - 42, start.size.w, start.size.h);
  animate(text_layer_get_layer(s_p_layer), start, finish, 400, 200);

  start = layer_get_frame(text_layer_get_layer(s_g_layer));
  finish = GRect(start.origin.x, start.origin.y - 42, start.size.w, start.size.h);
  animate(text_layer_get_layer(s_g_layer), start, finish, 400, 350);

  start = layer_get_frame(text_layer_get_layer(s_e_layer));
  finish = GRect(start.origin.x, start.origin.y - 42, start.size.w, start.size.h);
  animate(text_layer_get_layer(s_e_layer), start, finish, 400, 500);

  // Schedule hide
  app_timer_register(1500, hide_callback, NULL);
}

// Override back to avoid unfinished animations
// TODO Need a better solution to this
static void back_click_handler(ClickRecognizerRef recognizer, void *context) { }

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void splash_window_load(Window *window) {
  // Cache Layer
  Layer *window_layer = window_get_root_layer(window);

  // Built with...
  s_built_layer = text_layer_create(GRect(0, 40, 144, 50));
  text_layer_set_font(s_built_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_color(s_built_layer, GColorWhite);
  text_layer_set_background_color(s_built_layer, GColorClear);
  text_layer_set_text_alignment(s_built_layer, GTextAlignmentCenter);
  text_layer_set_text(s_built_layer, "Built with");
  layer_add_child(window_layer, text_layer_get_layer(s_built_layer));

  // P, G, E layers
  s_p_layer = text_layer_create(GRect(30, SPLASH_LOGO_Y, 30, 50));
  text_layer_set_font(s_p_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_color(s_p_layer, GColorWhite);
  text_layer_set_background_color(s_p_layer, GColorClear);
  text_layer_set_text(s_p_layer, "P");
  layer_add_child(window_layer, text_layer_get_layer(s_p_layer));

  s_g_layer = text_layer_create(GRect(55, SPLASH_LOGO_Y, 30, 50));
  text_layer_set_font(s_g_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_color(s_g_layer, GColorWhite);
  text_layer_set_background_color(s_g_layer, GColorClear);
  text_layer_set_text(s_g_layer, "G");
  layer_add_child(window_layer, text_layer_get_layer(s_g_layer));

  s_e_layer = text_layer_create(GRect(90, SPLASH_LOGO_Y, 30, 50));
  text_layer_set_font(s_e_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_color(s_e_layer, GColorWhite);
  text_layer_set_background_color(s_e_layer, GColorClear);
  text_layer_set_text(s_e_layer, "E");
  layer_add_child(window_layer, text_layer_get_layer(s_e_layer));

  // Dividing line, start compressed
  s_line_layer = text_layer_create(GRect(0, SPLASH_LOGO_Y, 0, SPLASH_LINE_HEIGHT));
  text_layer_set_background_color(s_line_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_line_layer));

  // Block layer to hide letters - do last!
  s_block_layer = text_layer_create(GRect(0, (SPLASH_LOGO_Y) + SPLASH_LINE_HEIGHT, 144, 80));
  text_layer_set_background_color(s_block_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(s_block_layer));
}

static void splash_window_unload(Window *window) {
  // Destroy line Layer
  text_layer_destroy(s_line_layer);
  text_layer_destroy(s_block_layer);
  text_layer_destroy(s_p_layer);
  text_layer_destroy(s_g_layer);
  text_layer_destroy(s_e_layer);
  text_layer_destroy(s_built_layer);

  // Destroy Window
  window_destroy(s_splash_window);
  s_splash_window = NULL;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "splash unloaded");
}

static void splash_window_appear(Window *window) {
  // If returning, pop immediately
  if(!appeared) {
    appeared = true;
  } else {
    window_stack_pop(false);
  }
}

static void splash_init() {
  // Create Window
  s_splash_window = window_create();
  window_set_background_color(s_splash_window, GColorBlack);
#if defined(PBL_SDK_2)
  window_set_fullscreen(s_splash_window, true);
#endif
  window_set_click_config_provider(s_splash_window, click_config_provider);
  window_set_window_handlers(s_splash_window, (WindowHandlers) {
    .load = splash_window_load, 
    .unload = splash_window_unload,
    .appear = splash_window_appear
  });
  window_stack_push(s_splash_window, true);

  // Begin animations
  start_animations();
}

void pge_splash_show(PGESplashDoneHandler *handler) {
  // Rate limit
  if(s_splash_window == NULL) {
    s_done_handler = handler;
    splash_init();
  }
}