#include "main_window.h"

// Use C-D-Lewis/pico-dev repo pico-pipes project to compile this from a MIDI file
#include "../notes-pebble.h"

static Window *s_window;
static TextLayer *s_progress_layer;

// TODO: Use this to visualize progress through the song
static AppTimer *s_note_timer;
static int s_note_index = 0;

static void play_notes() {
  speaker_play_tracks(TRACKS, ARRAY_LENGTH(TRACKS), 80);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(root_layer);

  s_progress_layer = text_layer_create(bounds);
  text_layer_set_font(s_progress_layer, scl_get_font(SFI_Medium));
  layer_add_child(root_layer, text_layer_get_layer(s_progress_layer));

  text_layer_set_text(s_progress_layer, "Ready");
}

static void window_unload(Window *window) {
  text_layer_destroy(s_progress_layer);

  window_destroy(window);
  s_window = NULL;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Restart?
  
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Start
  play_notes();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

void main_window_push() {
  if (!s_window) {
    s_window = window_create();
    window_set_click_config_provider(s_window, click_config_provider);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(s_window, true);
}
