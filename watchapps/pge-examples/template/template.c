/**
 * This is the bare minimum to make a looping game with PGE!
 */

#include <pebble.h>

#include "pge/pge.h"

static Window *s_game_window;

static void game_logic() {
  // Per-frame game logic here

}

static void game_draw(GContext *ctx) {
  // Per-frame game rendering here

}

static void game_click(int button_id, bool long_click) {
  // Process click events

}

void pge_init() {
  // Start the game
  pge_begin(game_logic, game_draw, game_click);

  // Keep a Window reference for adding other UI
  s_game_window = pge_get_window();
}

void pge_deinit() {
  // Finish the game
  pge_finish();
}