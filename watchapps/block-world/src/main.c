#include <pebble.h>

#include "main.h"
#include "pge/modules/pge_isometric.h"
#include "pge/pge.h"
#include "drawable/block.h"
#include "drawable/cloud.h"

int g_angle = 0;

static Window *s_main_window;
static TextLayer *s_status_layer;

static Block *s_block_array[GRID_WIDTH * GRID_HEIGHT * GRID_DEPTH];
static Cloud *s_cloud_array[MAX_CLOUDS];

static Vec3 s_cursor;
static int s_input_mode = MODE_X;
static int s_block_choice = BLOCK_GRASS;

static void update_status_text();

#ifdef BENCHMARK
static int get_world_size() {
  return sizeof(s_block_array) * block_get_struct_size();
}
#endif

static int vec2i(Vec3 vec) {
  if(vec.x >= 0 
  && vec.x < GRID_WIDTH 
  && vec.y >= 0 
  && vec.y < GRID_HEIGHT
  && vec.z >= 0
  && vec.z < GRID_DEPTH
  ) {
    return (vec.z * (GRID_WIDTH * GRID_HEIGHT)) + (vec.y * GRID_HEIGHT) + vec.x;
  } else {
    return -1;
  }
}

static void logic() {
  // Spawn cloud?
  if(rand() % 100 > CLOUD_SPAWN_CHANCE) {
    // Find invisible cloud
    int i = 0;
    for(i = 0; i < MAX_CLOUDS; i++) {
      if(!cloud_get_visible(s_cloud_array[i])) {
        cloud_spawn(s_cloud_array[i]);
        break;
      }
    }
  }

  // Update exisiting
  for(int i = 0; i < MAX_CLOUDS; i++) {
    cloud_logic(s_cloud_array[i]);
  }

  g_angle += 2 * (TRIG_MAX_ANGLE / 360);
}

static void render(GContext *ctx) {
#ifdef BENCHMARK
  uint16_t start = time_ms(NULL, NULL);
#endif

  pge_isometric_begin(ctx);

  // Tiles
  for(int z = 0; z < GRID_DEPTH; z++) {
    for(int y = 0; y < GRID_HEIGHT; y++) {
      for(int x = 0; x < GRID_WIDTH; x++) {
        block_render(s_block_array[vec2i(Vec3(x, y, z))]);
      }
    }
  }

  for(int i = 0; i < MAX_CLOUDS; i++) {
    cloud_render(s_cloud_array[i]);
  }

  // Selection
  switch(s_input_mode) {
    case MODE_X:
      pge_isometric_draw_rect(Vec3(0, s_cursor.y * BLOCK_SIZE, (s_cursor.z + 1) * BLOCK_SIZE), GSize(GRID_WIDTH * BLOCK_SIZE, BLOCK_SIZE), GColorWhite);
      break;
    case MODE_Y:
      pge_isometric_draw_rect(Vec3(s_cursor.x * BLOCK_SIZE, 0, (s_cursor.z + 1) * BLOCK_SIZE), GSize(BLOCK_SIZE, GRID_HEIGHT * BLOCK_SIZE), GColorWhite);
      break;
    case MODE_Z:
      pge_isometric_draw_box(Vec3(s_cursor.x * BLOCK_SIZE, s_cursor.y * BLOCK_SIZE,  BLOCK_SIZE), GSize(BLOCK_SIZE, BLOCK_SIZE), (GRID_DEPTH - 1) * BLOCK_SIZE, GColorWhite);
      break;
  }
  pge_isometric_draw_box(Vec3(s_cursor.x * BLOCK_SIZE, s_cursor.y * BLOCK_SIZE, s_cursor.z * BLOCK_SIZE), GSize(BLOCK_SIZE, BLOCK_SIZE), BLOCK_SIZE, GColorRed);

  // Box
  pge_isometric_draw_box(Vec3(0, 0, 0), GSize(BLOCK_SIZE * GRID_WIDTH, BLOCK_SIZE * GRID_HEIGHT), SKY_HEIGHT, GColorLightGray);

  pge_isometric_finish(ctx);

#ifdef BENCHMARK
  uint16_t finish = time_ms(NULL, NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Frame time: %d ms", (int)finish - start);
#endif
}

static void click(int button_id, bool long_click) {
  switch(button_id) {
    case BUTTON_ID_UP:
      s_input_mode = (s_input_mode + 1) % 4;
      break;
    case BUTTON_ID_SELECT:
      switch(s_block_choice) {
        case BLOCK_GRASS:
          block_set_color(s_block_array[vec2i(s_cursor)], COLOR_GRASS);
          break;
        case BLOCK_SAND:
          block_set_color(s_block_array[vec2i(s_cursor)], COLOR_SAND);
          break;
        case BLOCK_WATER:   
          block_set_color(s_block_array[vec2i(s_cursor)], COLOR_WATER);
          break;
        case BLOCK_RED:
          block_set_color(s_block_array[vec2i(s_cursor)], COLOR_RED);
          break;
        case BLOCK_STONE:
          block_set_color(s_block_array[vec2i(s_cursor)], COLOR_STONE);
          break;
        case BLOCK_DELETE:
          block_set_color(s_block_array[vec2i(s_cursor)], COLOR_INVISIBLE);
          break;
      }
      
      break;
    case BUTTON_ID_DOWN:
      switch(s_input_mode) {
        case MODE_X:
          s_cursor.x = (s_cursor.x + 1) % GRID_WIDTH;
          break;
        case MODE_Y:
          s_cursor.y = (s_cursor.y + 1) % GRID_HEIGHT;
          break;
        case MODE_Z:
          s_cursor.z = (s_cursor.z + 1) % GRID_DEPTH;
          break;
        case MODE_BLOCK_CHOICE:
          s_block_choice = (s_block_choice + 1) % NUM_BLOCK_TYPES;
          break;
      }
      break;
  }

  update_status_text();
  pge_manual_advance();
}

static bool is_near_color(GPoint origin, int z, GColor query) {
  for(int y = origin.y - 1; y < origin.y + 2; y++) {
    for(int x = origin.x -1; x < origin.x + 2; x++) {
      // Check only on this level
      int checked_point = vec2i(Vec3(x, y, z));
      if(checked_point > 0) {
        if(block_get_color(s_block_array[checked_point]).argb == query.argb) {
          return true;
        }
      }
    }
  }

  return false;
}

static bool near_this_many_of_color(GPoint origin, int z, GColor query, int num) {
  int result = 0;
  for(int y = origin.y - 1; y < origin.y + 2; y++) {
    for(int x = origin.x -1; x < origin.x + 2; x++) {
      // Check only on this level
      int checked_point = vec2i(Vec3(x, y, z));
      if(checked_point > 0) {
        if(block_get_color(s_block_array[checked_point]).argb == query.argb) {
          result++;
        }
      }
    }
  }

  return (result >= num);
}

static void generate_world() {
  srand(time(NULL));

#ifdef BENCHMARK
  uint16_t start = time_ms(NULL, NULL);
#endif

  // Bottom level grass
  for(int y = 0; y < GRID_HEIGHT; y++) {
    for(int x = 0; x < GRID_WIDTH; x++) {
      block_set_color(s_block_array[vec2i(Vec3(x, y, 0))], COLOR_GRASS);
    }
  }

  // Seed sand
  for(int y = 0; y < GRID_HEIGHT; y++) {
    for(int x = 0; x < GRID_WIDTH; x++) {
      if(rand() % 100 > SAND_SEED_BIAS) {
        block_set_color(s_block_array[vec2i(Vec3(x, y, 0))], COLOR_SAND);
      }
    }
  }

  // Clump sand
  for(int y = 0; y < GRID_HEIGHT; y++) {
    for(int x = 0; x < GRID_WIDTH; x++) {
      if(rand() % 100 > SAND_CLUMP_BIAS
      && is_near_color(GPoint(x, y), 0, COLOR_SAND)) {
        block_set_color(s_block_array[vec2i(Vec3(x, y, 0))], COLOR_SAND);
      }
    }
  }
  for(int y = 0; y < GRID_HEIGHT; y++) {
    for(int x = 0; x < GRID_WIDTH; x++) {
      if(near_this_many_of_color(GPoint(x, y), 0, COLOR_SAND, SAND_NEIGHBOURS)) {
        block_set_color(s_block_array[vec2i(Vec3(x, y, 0))], COLOR_SAND);
      }
    }
  }


  // Water in sand areas
  for(int y = 0; y < GRID_HEIGHT; y++) {
    for(int x = 0; x < GRID_WIDTH; x++) {
      if(near_this_many_of_color(GPoint(x, y), 0, COLOR_SAND, WATER_NEIGHBOURS)) {
        block_set_color(s_block_array[vec2i(Vec3(x, y, 0))], COLOR_WATER);
      }
    }
  }

  // Clump water
  for(int y = 0; y < GRID_HEIGHT; y++) {
    for(int x = 0; x < GRID_WIDTH; x++) {
      if(rand() % 100 > WATER_CLUMP_BIAS
      && is_near_color(GPoint(x, y), 0, COLOR_WATER)) {
        block_set_color(s_block_array[vec2i(Vec3(x, y, 0))], COLOR_WATER);
      }
    }
  }

  // Castle walls
  int x = 2;
  int y = 6;
  for(y = 6; y < 10; y++) {
    for(int z = 0; z < 6; z++) {
      block_set_color(s_block_array[vec2i(Vec3(x, y, z))], COLOR_STONE);
    }
  }
  y = 11;
  for(x = 2; x < 6; x++) {
    for(int z = 0; z < 6; z++) {
      block_set_color(s_block_array[vec2i(Vec3(x, y, z))], COLOR_STONE);
    }
  }
  y = 6;
  for(x = 2; x < 6; x++) {
    for(int z = 0; z < 6; z++) {
      block_set_color(s_block_array[vec2i(Vec3(x, y, z))], COLOR_STONE);
    }
  }
  x = 6;
  for(y = 6; y < 10; y++) {
    for(int z = 0; z < 6; z++) {
      block_set_color(s_block_array[vec2i(Vec3(x, y, z))], COLOR_STONE);
    }
  }
  int z = 6;
  for(y = 6; y < 10; y++) {
    for(x = 2; x < 7; x++) {
      block_set_color(s_block_array[vec2i(Vec3(x, y, z))], COLOR_STONE);
    }
  }

#ifdef BENCHMARK
  uint16_t finish = time_ms(NULL, NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Generation time: %d ms", (int)finish - start);
#endif
}

static char* get_block_name() {
  switch(s_block_choice) {
    case BLOCK_GRASS: return "B:GRASS";
    case BLOCK_SAND: return "B:SAND";
    case BLOCK_WATER: return "B:WATER";
    case BLOCK_RED: return "B:RED";
    case BLOCK_STONE: return "B:STONE";
    case BLOCK_DELETE: return "B:DELETE";
    default: return "B:UNKN";
  }
}

static char* get_mode_string() {
  switch(s_input_mode) {
    case MODE_X: return "M:X";
    case MODE_Y: return "M:Y";
    case MODE_Z: return "M:Z";
    case MODE_BLOCK_CHOICE: return "M:B";
    default: return "M:UNKN";
  }
}

static void update_status_text() {
  static char s_buff[32];
  snprintf(s_buff, sizeof(s_buff), "(%d,%d,%d) | %s | %s", 
    s_cursor.x, s_cursor.y, s_cursor.z, get_mode_string(), get_block_name()
  );
  text_layer_set_text(s_status_layer, s_buff);
}

void pge_init() {
  // Allocate
  for(int z = 0; z < GRID_DEPTH; z++) {
    for(int y = 0; y < GRID_HEIGHT; y++) {
      for(int x = 0; x < GRID_WIDTH; x++) {
        s_block_array[vec2i(Vec3(x, y, z))] = block_create(Vec3(x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE), GSize(BLOCK_SIZE, BLOCK_SIZE), COLOR_INVISIBLE);
      }
    }
  }
  for(int i = 0; i < MAX_CLOUDS; i++) {
    s_cloud_array[i] = cloud_create(Vec3(0, 0, SKY_HEIGHT), GSize(BLOCK_SIZE, BLOCK_SIZE), Vec3(GRID_WIDTH * BLOCK_SIZE, GRID_HEIGHT * BLOCK_SIZE, SKY_HEIGHT));
  }

  // Set up world
  generate_world();

  // Set up engine
  pge_isometric_set_projection_offset(PBL_IF_ROUND_ELSE(GPoint(90, 110), GPoint(72, 80)));
  pge_isometric_set_enabled(true);
  pge_set_framerate(FRAME_RATE_IDLE);
  pge_begin(GColorBlack, logic, render, click);
  s_main_window = pge_get_window();

  s_status_layer = text_layer_create(grect_inset(
    layer_get_bounds((window_get_root_layer(s_main_window))),
    PBL_IF_ROUND_ELSE(GEdgeInsets(30, 0, 130, 0), GEdgeInsets(0, 0, 150, 0))));
  text_layer_set_background_color(s_status_layer, GColorBlack);
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_text_alignment(s_status_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_status_layer));
  update_status_text();

#ifdef BENCHMARK
  APP_LOG(APP_LOG_LEVEL_INFO, "Heap free: %dB after creating %d blocks (Size: %dB)", (int)heap_bytes_free(), (GRID_WIDTH * GRID_HEIGHT * GRID_DEPTH), get_world_size());
#endif
}

void pge_deinit() {
  pge_finish();

  text_layer_destroy(s_status_layer);

  // Deallocate
  for(int z = 0; z < GRID_DEPTH; z++) {
    for(int y = 0; y < GRID_HEIGHT; y++) {
      for(int x = 0; x < GRID_WIDTH; x++) {
        block_destroy(s_block_array[vec2i(Vec3(x, y, z))]);
      }
    }
  }
  for(int i = 0; i < MAX_CLOUDS; i++) {
    cloud_destroy(s_cloud_array[i]);
  }
}