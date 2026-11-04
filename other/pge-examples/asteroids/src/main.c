#include "main.h"

// UI
static Window *s_game_window;
static TextLayer *s_score_layer;

// Game
static Ship *s_player = NULL;
static Shot *shot_arr[MAX_OBJECTS];
static Rock *rock_arr[MAX_OBJECTS];

// State
static char s_score_buffer[32];
static int s_lives = 3, s_score = 0;

/******************************** Game functions ******************************/

static void update_status_text() {
  snprintf(s_score_buffer, sizeof(s_score_buffer), "Lives: %d | Score: %d", s_lives, s_score);
  text_layer_set_text(s_score_layer, s_score_buffer);
}

static void fire() {
  GPoint pos = ship_get_position(s_player);
  GRect bounds = ship_get_bounds(s_player);

  // Adjust
  pos.x += bounds.size.w / 2;

  // Find slot
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(!shot_arr[i]) {
      shot_arr[i] = shot_create(pos);
      return;
    }
  }
}

static void spawn_rock() {
  GPoint pos = GPoint(rand() % 144, -20);
  if(pos.x > 144 - 30) {
    pos.x = 144 - 30;
  }

  // Find slot
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(!rock_arr[i]) {
      rock_arr[i] = rock_create(pos);
      return;
    }
  }
}

static void spawn_handler(void *context) {
  spawn_rock();

  // Continue loop
  app_timer_register(rand() % MAX_SPAWN_INTERVAL, spawn_handler, NULL);
}

/******************************** Engine callbacks ****************************/

static void logic() {
  GPoint pos = ship_get_position(s_player);
  GRect bounds = ship_get_bounds(s_player);

  // Continuous movement
  if(pge_get_button_state(BUTTON_ID_UP) && pos.x > 0) {
    ship_move(s_player, -SHIP_MOVE_SPEED);
  } else if(pge_get_button_state(BUTTON_ID_DOWN) && pos.x < (144 - bounds.size.w)) {
    ship_move(s_player, SHIP_MOVE_SPEED);
  }

  // Move shots
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(shot_arr[i]) {
      shot_logic(shot_arr[i]);

      // Out of bounds?
      if(shot_get_position(shot_arr[i]).y < 0) {
        shot_destroy(shot_arr[i]);
        shot_arr[i] = NULL;
      }
    }
  }

  // Move rocks
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(rock_arr[i]) {
      rock_logic(rock_arr[i]);

      // Out of bounds of bottom of screen?
      if(rock_get_position(rock_arr[i]).y > 168) {
        rock_destroy(rock_arr[i]);
        rock_arr[i] = NULL;
      }
    }
  }

  // Do shot collision with rocks
  for(int s = 0; s < MAX_OBJECTS; s++) {
    for(int r = 0; r < MAX_OBJECTS; r++) {
      if(rock_arr[r] && shot_arr[s]) {
        if(pge_check_collision(rock_arr[r]->sprite, shot_arr[s]->sprite)) {
          //Boom!
          rock_destroy(rock_arr[r]);
          shot_destroy(shot_arr[s]);
          rock_arr[r] = NULL;
          shot_arr[s] = NULL;

          // Award points
          s_score++;
          update_status_text();
        }
      }
    }
  }

  // Do rock collision with player
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(rock_arr[i]) {
      if(pge_check_collision(s_player->sprite, rock_arr[i]->sprite)) {
        // Lose a life
        s_lives--;
        update_status_text();

        vibes_long_pulse();

        // Destroy rock
        rock_destroy(rock_arr[i]);
        rock_arr[i] = NULL;
      }
    }
  }

  if(s_lives == 0) {
    window_stack_pop_all(true);
    return;
  }

}

static void draw(GContext *ctx) {
  ship_draw(s_player, ctx);

  // Draw any active shots
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(shot_arr[i]) {
      shot_draw(shot_arr[i], ctx);
    }
  }

  // Draw any active rocks
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(rock_arr[i]) {
      rock_draw(rock_arr[i], ctx);
    }
  }
}

static void click(int button_id, bool long_click) {
  switch(button_id) {
    case BUTTON_ID_SELECT:
      // Fire! Shots will self destroy
      fire();
      break;
  }
}

/********************************* App Foundation *****************************/

static void splash_done_handler() {
  // Create player's Ship
  s_player = ship_create(GPoint(60, 130));

  // Begin game loop
  pge_begin(logic, draw, click);
  s_game_window = pge_get_window();
  pge_set_framerate(25);

  // Create score layer
  s_score_layer = text_layer_create(GRect(0, 0, 144, 20));
  text_layer_set_text_color(s_score_layer, GColorWhite);
  text_layer_set_background_color(s_score_layer, GColorBlack);
  text_layer_set_font(s_score_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(s_game_window), text_layer_get_layer(s_score_layer));  

  // Setup score
  update_status_text();

  // Begin spawn loop
  app_timer_register(rand() % MAX_SPAWN_INTERVAL, spawn_handler, NULL);
}

void pge_init() {
  srand(time(NULL));

  pge_splash_show(splash_done_handler);
}

void pge_deinit() {
  // Destroy score layer
  text_layer_destroy(s_score_layer);

  //Free Shots and Rocks
  for(int i = 0; i < MAX_OBJECTS; i++) {
    if(shot_arr[i]) {
      shot_destroy(shot_arr[i]);
    }
    if(rock_arr[i]) {
      rock_destroy(rock_arr[i]);
    }
  }

  // Destroy the player's Ship
  if(s_player) {
    ship_destroy(s_player);
  }

  // End game loop
  pge_finish();
}