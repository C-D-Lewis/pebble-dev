#include <pebble.h>

#include <pebble-pge-simple/pebble-pge-simple.h>
#include <pebble-isometric/pebble-isometric.h>

#include "drawable/block.h"

#define NUM_BLOCKS 4
#define MAX_HEIGHT 90
#define DURATION  2000

static Block* s_blocks[NUM_BLOCKS];

static int distance2z(AnimationProgress progress) {
  int result = (progress * MAX_HEIGHT) / ANIMATION_NORMALIZED_MAX;
  return result < MAX_HEIGHT ? result : MAX_HEIGHT;
}

static void up_update_implementation(Animation *animation, const AnimationProgress progress) {
  int extra = progress / 5;

  block_set_z(s_blocks[3], distance2z(progress));
  block_set_z(s_blocks[2], distance2z(progress + extra));
  block_set_z(s_blocks[1], distance2z(progress + (2 * extra)));
  block_set_z(s_blocks[0], distance2z(progress + (3 * extra)));
}

static void down_update_implementation(Animation *animation, const AnimationProgress progress) {
  int extra = progress / 10;
  
  block_set_z(s_blocks[3], MAX_HEIGHT - (distance2z(progress)));
  block_set_z(s_blocks[2], MAX_HEIGHT - (distance2z(progress + extra)));
  block_set_z(s_blocks[1], MAX_HEIGHT - (distance2z(progress + (2 * extra))));
  block_set_z(s_blocks[0], MAX_HEIGHT - (distance2z(progress + (3 * extra))));
}

static void game_logic() {
  // Per-frame game logic here

}

static void game_draw(GContext *ctx) {
  // Per-frame game rendering here

  isometric_begin(ctx);
  for(int i = 0; i < NUM_BLOCKS; i++) {
    block_render(s_blocks[i]);
  }
  isometric_finish(ctx);
}

static void game_click(int button_id, bool long_click) {
  // Process click events
  APP_LOG(APP_LOG_LEVEL_INFO, "FPS: %d", pge_get_average_framerate());
}

void pge_init() {
  // Create blocks
  s_blocks[0] = block_create(Vec3(BLOCK_HEIGHT, BLOCK_HEIGHT, 0));
  block_set_colors(s_blocks[0], GColorDukeBlue, GColorBlue);
  s_blocks[1] = block_create(Vec3(2 * BLOCK_HEIGHT, BLOCK_HEIGHT, 0));
  block_set_colors(s_blocks[1], GColorDarkCandyAppleRed, GColorRed);
  s_blocks[2] = block_create(Vec3(BLOCK_HEIGHT, 2 * BLOCK_HEIGHT, 0));
  block_set_colors(s_blocks[2], GColorDarkGreen, GColorGreen);
  s_blocks[3] = block_create(Vec3(2 * BLOCK_HEIGHT, 2 * BLOCK_HEIGHT, 0));
  block_set_colors(s_blocks[3], GColorChromeYellow, GColorYellow);

  // Start the game
  pge_begin(game_logic, game_draw, game_click);
  isometric_set_projection_offset(PBL_IF_ROUND_ELSE(GPoint(90, 105), GPoint(70, 100)));

  // Animate blocks
  static AnimationImplementation up_impl = {
    .update = up_update_implementation
  };
  Animation *up_anim = animation_create();
  animation_set_duration(up_anim, DURATION);
  animation_set_curve(up_anim, AnimationCurveEaseInOut);
  animation_set_implementation(up_anim, &up_impl);
  static AnimationImplementation down_impl = {
    .update = down_update_implementation
  };
  Animation *down_anim = animation_create();
  animation_set_duration(down_anim, DURATION);
  animation_set_curve(down_anim, AnimationCurveEaseInOut);
  animation_set_implementation(down_anim, &down_impl);

  Animation *anim = animation_sequence_create(up_anim, down_anim, NULL);
  animation_set_play_count(anim, ANIMATION_PLAY_COUNT_INFINITE);
  animation_schedule(anim);
}

void pge_deinit() {
  // Finish the game
  pge_finish();

  // Destroy blocks
  for(int i = 0; i < NUM_BLOCKS; i++) {
    block_destroy(s_blocks[i]);
  }
}