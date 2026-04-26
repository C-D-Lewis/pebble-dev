#include "util.h"

#if defined(PBL_PLATFORM_EMERY)
  #define CLOUD_SIZE GSize(30, 19)
#elif defined(PBL_PLATFORM_CHALK)
  #define CLOUD_SIZE GSize(24, 15)
#else
  #define CLOUD_SIZE GSize(24, 15)
#endif

TextLayer* util_make_text_layer(GRect frame, GFont font) {
  TextLayer *this = text_layer_create(frame);
  text_layer_set_font(this, font);
  text_layer_set_background_color(this, GColorClear);
  return this;
}

void util_fmt_time(int timestamp_s, char* buff, int size) {
  if (timestamp_s < 0) {
    snprintf(buff, size, "-");
    return;
  }

  const time_t t = (time_t)timestamp_s;
  const struct tm *tm_info = localtime(&t);
  strftime(buff, size, "%H:%M", tm_info);
}

void util_fmt_time_ago(time_t then, char *buff, int size) {
  int diff_s = time(NULL) - then;

  // Fudge: always a positive amount of time for display purposes
  diff_s *= diff_s < 0 ? -1 : 1;

  int value = 0;
  const char *unit;
  if (diff_s < 60) {
    value = diff_s;
    unit = "s";
  } else if (diff_s < 3600) {
    value = diff_s / 60;
    unit = "m";
  } else if (diff_s < SECONDS_PER_DAY) {
    value = diff_s / 3600;
    unit = "h";
  } else {
    value = diff_s / SECONDS_PER_DAY;
    unit = "d";
  }

  snprintf(buff, size, "%d%s", value, unit);
}

void util_fmt_time_unit(time_t ts, char *buff, int size) {
  int value = 0;
  const char *unit;

  // TODO: DRY this out with util_fmt_time_ago()
  if (ts < 60) {
    value = ts;
    unit = "s";
  } else if (ts < 3600) {
    value = ts / 60;
    unit = "m";
  } else if (ts < SECONDS_PER_DAY) {
    value = ts / 3600;
    unit = "h";
  } else {
    value = ts / SECONDS_PER_DAY;
    unit = "d";
  }

  snprintf(buff, size, "%d%s", value, unit);
}

char* util_get_status_string() {
  PersistData *persist_data = data_get_persist_data();

  const bool is_enabled = util_is_not_status(persist_data->wakeup_id);
  if (!is_enabled) return "Not monitoring";

  // No readings at all yet
  if (!util_is_not_status(persist_data->last_sample_time)) return "Awaiting sample...";

  // Edge case here: no change or charging samples don't count
  // We can't produce a prediction AT ALL unless we have 'discharging' samples...
  const int count = data_get_valid_samples_count();
  if (count < MIN_SAMPLES) {
    const int diff = MIN_SAMPLES - count;
    static char s_buff[20];
    snprintf(s_buff, sizeof(s_buff), "Need %d sample%s...", diff, diff == 1 ? "" : "s");
    return &s_buff[0];
  }

  // Ongoing readings
  return "Passively monitoring";
}

uint32_t util_get_battery_resource_id(int charge_percent) {
  if (charge_percent > 66) return RESOURCE_ID_BATTERY_HIGH;
  if (charge_percent > 33) return RESOURCE_ID_BATTERY_MEDIUM;
  return RESOURCE_ID_BATTERY_LOW;
}

bool util_is_not_status(int v) {
  return v != STATUS_EMPTY && v != STATUS_CHARGED && v != STATUS_NO_CHANGE;
}

bool util_is_not_charging(int v) {
  // Allow result and STATUS_NO_CHANGE (still plugged in)
  return util_is_not_status(v) || v == STATUS_NO_CHANGE;
}

void util_draw_braid(GContext *ctx, GRect rect) {
// #if !defined(PBL_PLATFORM_APLITE)
  graphics_draw_bitmap_in_rect(ctx, bitmaps_get(RESOURCE_ID_BRAID), rect);
// #else
//   graphics_context_set_fill_color(ctx, GColorBlack);
//   graphics_fill_rect(ctx, rect, 0, GCornerNone);

//   // Draw braid without image on Aplite to save memory
//   const uint8_t size = 2;
//   const uint8_t end_x = rect.size.w - size;
//   const uint8_t start_y = rect.origin.y + size;
//   const uint8_t end_y = rect.origin.y + rect.size.h - size;
//   for (uint8_t y = start_y; y < end_y; y += size) {
//     const bool offset = ((y - start_y) / size) % 2 == 1;
//     for (uint8_t x = size + (offset ? size : 0); x < end_x; x += (2 * size)) {
//       graphics_context_set_fill_color(ctx, GColorWhite);
//       graphics_fill_rect(ctx, GRect(x, y, size, size), 0, GCornerNone);
//     }
//   }
// #endif
}

// Like menu_cell_basic_draw but with larger subtitle
void util_menu_cell_draw(GContext *ctx, Layer *layer, char *title, char *desc) {
  // TODO: Can we use ContentSize here without layout issues?
  //       It may conflict with pebble-scalable font system
  //
  // Somehow removing this consumes 200B more static memory!?
  PreferredContentSize content_size = preferred_content_size();
  // APP_LOG(APP_LOG_LEVEL_INFO, "content_size: %d", (int)content_size);

  // Medium or smaller (rare?), use regular rendering
  if (content_size <= PreferredContentSizeMedium) {
    menu_cell_basic_draw(ctx, layer, title, desc, NULL);
    return;
  }

  // Else, use larger one
  GRect title_rect = GRect(scl_x(30), scl_y_pp({.o = -30, .e = -10}), PS_DISP_W, 100);
  if (desc == NULL) {
    title_rect.origin.y += scl_y(30);
  }

  graphics_draw_text(
    ctx,
    title,
    scl_get_font(SFI_MediumBold),
    title_rect,
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );

  if (desc != NULL) {
    graphics_draw_text(
      ctx,
      desc,
      scl_get_font(SFI_Medium),
      GRect(scl_x(30), scl_y_pp({.o = 110, .e = 130}), PS_DISP_W, 100),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentLeft,
      NULL
    );
  }
}

void util_draw_button_hints(GContext *ctx, bool hints[3]) {
  // Actions BG
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkCandyAppleRed, GColorLightGray));
  GRect actions_rect = GRect(PS_DISP_W - ACTION_BAR_W, 0, ACTION_BAR_W, PS_DISP_H);
  graphics_fill_rect(ctx, actions_rect, 0, GCornerNone);

#if !defined(PBL_PLATFORM_CHALK)
  const int hint_x = PS_DISP_W - (HINT_W / 2);
#else
  const int hint_x = PS_DISP_W - ((2 * HINT_W) / 3);
#endif

  if (hints[0]) {
    // Top hint
#if !defined(PBL_PLATFORM_CHALK)
    const int top_y = PS_DISP_H / 6 - (HINT_H / 2);
#else
    const int top_y = ((3 * PS_DISP_H) / 8) - (HINT_H / 2);
#endif
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(hint_x, top_y, HINT_W, HINT_H), 3, GCornersAll);

    // TODO: Ugly, can't configure long press emphasis but only used in one place for now.
    graphics_context_set_fill_color(ctx, GColorWhite);
    int select_x = hint_x + (HINT_W / 2);
#ifdef PBL_PLATFORM_CHALK
    select_x -= 2;
#endif
    const GPoint select_center = { .x = select_x, .y = top_y + (HINT_H / 2) };
    graphics_fill_circle(ctx, select_center, scl_x_pp({.o = 20, .c = 10}));
  }

  if (hints[1]) {
    // Middle hint
#if !defined(PBL_PLATFORM_CHALK)
    const int middle_y = (PS_DISP_H / 2) - (HINT_H / 2);
#else
    const int middle_y = (PS_DISP_H / 2) - (HINT_H / 2);
#endif
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(hint_x, middle_y, HINT_W, HINT_H), 3, GCornersAll);
  }

  if (hints[2]) {
    // Bottom hint
#if !defined(PBL_PLATFORM_CHALK)
    const int bottom_y = ((5 * PS_DISP_H) / 6) - (HINT_H / 2);
#else
    const int bottom_y = ((5 * PS_DISP_H) / 8) - (HINT_H / 2);
#endif
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(hint_x, bottom_y, HINT_W, HINT_H), 3, GCornersAll);
  }
}

void util_draw_skyline(GContext *ctx, bool is_nighttime) {
  // Mascot banner
  // const GColor day_color = PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite);
  // const GColor night_color = PBL_IF_COLOR_ELSE(GColorOxfordBlue, GColorBlack);
  // graphics_context_set_fill_color(ctx, is_nighttime ? night_color : day_color);
  graphics_context_set_fill_color(ctx, is_nighttime ? GColorBlack : GColorWhite);
  const uint8_t skyline_y = scl_y(160);
  const GRect skyline_rect = GRect(0, 0, PS_DISP_W, skyline_y);
  graphics_fill_rect(ctx, skyline_rect, 0, GCornerNone);

  if (is_nighttime) {
    graphics_context_set_fill_color(ctx, GColorWhite);
    // Draw a couple of stars in black
    const GPoint stars[] = {
      GPoint(scl_x(40), scl_y(50)),
      GPoint(scl_x(90), scl_y(110)),
      GPoint(scl_x(300), scl_y(80)),
      GPoint(scl_x(200), scl_y(50)),
      GPoint(scl_x(800), scl_y(70)),
      GPoint(scl_x(720), scl_y(90)),
      GPoint(scl_x(610), scl_y(30)),
      GPoint(scl_x(880), scl_y(110))
    };
    const uint8_t num_stars = 8;
    const uint8_t star_max_size = scl_x(15);
    for (int i = 0; i < num_stars; i++) {
      const uint8_t size = (i % star_max_size) + 1;
      graphics_fill_rect(ctx, GRect(stars[i].x, stars[i].y, size, size), 0, GCornerNone);
    }

    // Skyline below mascot
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(
      ctx,
      GPoint(0, skyline_y),
      GPoint(PS_DISP_W - ACTION_BAR_W - 1, skyline_y)
    );
  } else {
    // Clouds and Huginn
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_CLOUD),
      GRect(scl_x_pp({.o = 60, .c = 240, .e = 80}), scl_y(20), CLOUD_SIZE.w, CLOUD_SIZE.h)
    );
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_CLOUD),
      GRect(scl_x(680), scl_y(50), CLOUD_SIZE.w, CLOUD_SIZE.h)
    );
#if !defined(PBL_PLATFORM_CHALK)
    graphics_draw_bitmap_in_rect(
      ctx,
      bitmaps_get(RESOURCE_ID_BIRD),
      GRect(scl_x(270), scl_y(30), 16, 16)
    );
#endif
  }
}

bool util_get_is_night() {
#ifdef TEST_IS_NIGHT
  return true;
#else
  const time_t now = time(NULL);
  const struct tm *now_info = localtime(&now);
  return now_info->tm_hour < 6 || now_info->tm_hour >= 18;
#endif
}

uint32_t util_get_mascot_res_id(bool is_enabled, bool is_night) {
  if (is_night) {
    return is_enabled ? RESOURCE_ID_AWAKE_HEAD_INV : RESOURCE_ID_ASLEEP_HEAD_INV;
  }

  return is_enabled ? RESOURCE_ID_AWAKE_HEAD : RESOURCE_ID_ASLEEP_HEAD;
}

static void underline_update_proc(Layer *layer, GContext *ctx) {
  // Title underline
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, scl_y(110), PS_DISP_W, LINE_W), 0, GCornerNone);

  // Draw title
  graphics_context_set_text_color(ctx, GColorBlack);
  char *title = (char*)layer_get_data(layer);
  graphics_draw_text(
    ctx,
    title,
    scl_get_font(SFI_Small),
    GRect(0, scl_y_pp({-30, .c = -20, .e = -25}), PS_DISP_W, 100),
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

Layer* util_create_header_layer(char *title, int title_size) {
  Layer *l = layer_create_with_data(GRect(0, 0, PS_DISP_W, 100), title_size);
  layer_set_update_proc(l, underline_update_proc);
  snprintf((char*)layer_get_data(l), title_size, "%s", title);
  return l;
}

////////////////////////////////////////// Animation Utils /////////////////////////////////////////

#ifdef FEATURE_ANIMATIONS
static Animation *s_animation;
static bool s_animating; // For now, we can assume only one is ever active

void util_stop_animation() {
  if (s_animation) {
    animation_unschedule(s_animation);
    animation_destroy(s_animation);
    s_animation = NULL;
  }
  s_animating = false;
}

int util_anim_percentage(AnimationProgress dist_normalized, int max) {
  return (max * dist_normalized) / ANIMATION_NORMALIZED_MAX;
}

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  util_stop_animation();
}

void util_animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  util_stop_animation();

  s_animation = animation_create();
  if (s_animation) {
    animation_set_duration(s_animation, duration);
    animation_set_delay(s_animation, delay);
    animation_set_curve(s_animation, AnimationCurveEaseOut);
    animation_set_implementation(s_animation, implementation);
    if (handlers) {
      animation_set_handlers(s_animation, (AnimationHandlers) {
        .started = animation_started,
        .stopped = animation_stopped
      }, NULL);
    }
    animation_schedule(s_animation);
  }
}

bool util_is_animating() {
  return s_animating;
}
#endif
