#include "main_window.h"

static GColor s_palette_pastel[] = {
  GColorVividCerulean,
  GColorMalachite,
  GColorYellow,
  GColorOrange,
  GColorRed,
  GColorVividViolet
};

static GColor s_palette_greyscale[] = {
  GColorDarkGray,
  GColorLightGray,
  GColorWhite,
  GColorDarkGray,
  GColorLightGray,
  GColorWhite,
};

static GColor s_palette_synthwave[] = {
  GColorOxfordBlue,
  GColorImperialPurple,
  GColorPurple,
  GColorMagenta,
  GColorFashionMagenta,
  GColorShockingPink,
};

static GColor s_palette_pride[] = {
  GColorRed,
  GColorOrange,
  GColorYellow,
  GColorVividCerulean,
  GColorIndigo,
  GColorMagenta
};

static GColor s_palette_forest[] = {
  GColorDarkGreen,
  GColorJaegerGreen,
  GColorMayGreen,
  GColorIslamicGreen,
  GColorWindsorTan,
  GColorArmyGreen
};

static GColor s_palette_ocean[] = {
  GColorOxfordBlue,
  GColorDukeBlue,
  GColorCobaltBlue,
  GColorPictonBlue,
  GColorElectricBlue,
  GColorWhite
};

static Window *s_window;
static Layer *s_canvas_layer;

static bool s_is_connected;

/*********************************** Drawing **********************************/

static GColor* get_current_palette() {
  char* palette_name = data_get_palette();
  if (strcmp(palette_name, "pastel") == 0)    return s_palette_pastel;
  if (strcmp(palette_name, "greyscale") == 0) return s_palette_greyscale;
  if (strcmp(palette_name, "synthwave") == 0) return s_palette_synthwave;
  if (strcmp(palette_name, "pride") == 0)     return s_palette_pride;
  if (strcmp(palette_name, "forest") == 0)    return s_palette_forest;
  if (strcmp(palette_name, "ocean") == 0)     return s_palette_ocean;

  return s_palette_pastel;
}

static void canvas_layer_update_proc(Layer *layer, GContext *ctx) {
  struct tm *tick_time;
  time_t t = time(NULL);
  tick_time = localtime(&t);

  isometric_begin(ctx);
  isometric_set_projection_offset(PROJECTION_OFFSET);

  GColor* palette = get_current_palette();
  if (!s_is_connected) {
    palette = s_palette_greyscale;
  }

  // Draw grid
  for (int y = 0; y < GRID_SIZE; y++) {
    for (int x = 0; x < GRID_SIZE; x++) {
      GColor color = palette[rand() % NUM_COLORS];
      int z_now = rand() % Z_MOD;
      drawing_draw_square(Vec3(x * B_W, y * B_W, z_now), color);
    }
  }

  isometric_finish(ctx);

  // Draw time
  static char s_time_buff[6];
  strftime(s_time_buff, sizeof(s_time_buff), "%H:%M", tick_time);
  graphics_context_set_text_color(ctx, GColorBlack);
  int x = scl_x_pp({.o = 20, .e = 25, .g = 20});
  int y = scl_y_pp({.o = 180, .e = 180, .g = 210});
  graphics_draw_text(
    ctx,
    s_time_buff,
    scl_get_font(SFI_Large),
    GRect(x + 3, y + 3, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_time_buff,
    scl_get_font(SFI_Large),
    GRect(x, y, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );

  // Date format '4 Apr 2026'
  static char s_date_buff[20];
  strftime(s_date_buff, sizeof(s_date_buff), "%e %b %Y", tick_time);

  x = scl_x_pp({.o = -10, .e = -10, .g = -5});
  y = scl_y_pp({.o = 550, .c = 530, .e = 550, .g = 530});
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
    ctx,
    s_date_buff,
    scl_get_font(SFI_Small),
    GRect(x + 2, y + 2, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(
    ctx,
    s_date_buff,
    scl_get_font(SFI_Small),
    GRect(x, y, PS_DISP_W, PS_DISP_H),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_canvas_layer);
}

static void bt_handler(bool connected) {
  if (connected == s_is_connected) return;

  s_is_connected = connected;

  if (!connected) vibes_double_pulse();

  layer_mark_dirty(s_canvas_layer);
}

/************************************ Window **********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_layer_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);

  window_destroy(s_window);
  s_window = NULL;
}

void main_window_push() {
  struct tm *tick_time;
  time_t t = time(NULL);
  tick_time = localtime(&t);
  srand(tick_time->tm_hour + tick_time->tm_min);

  if (!s_window) {
    s_window = window_create();
    window_set_background_color(s_window, GColorBlack);
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
  }
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  bluetooth_connection_service_subscribe(bt_handler);
  bt_handler(bluetooth_connection_service_peek());
}

void main_window_reload() {
  layer_mark_dirty(s_canvas_layer);
}
