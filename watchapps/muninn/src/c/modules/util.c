#include "util.h"

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
  const int hours = tm_info->tm_hour;
  const int mins = tm_info->tm_min;

  snprintf(buff, size, "%02d:%02d", hours, mins);
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

int util_hours_until_next_interval() {
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);

  int hour = tm_info->tm_hour;
  int rem = hour % WAKEUP_MOD_H;
  int hours = WAKEUP_MOD_H - rem;
  if (hours == 0) hours = WAKEUP_MOD_H;
  return hours;
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
    static char s_need_buff[20];
    snprintf(s_need_buff, sizeof(s_need_buff), "Need %d sample%s...", diff, diff == 1 ? "" : "s");
    return &s_need_buff[0];
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
  // Allow result and STATUS_NO_CHANGE
  return util_is_not_status(v) || v == STATUS_NO_CHANGE;
}

void util_draw_braid(GContext *ctx, GRect rect) {
#if !defined(PBL_PLATFORM_APLITE)
  graphics_draw_bitmap_in_rect(ctx, bitmaps_get(RESOURCE_ID_BRAID), rect);
#else
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, rect, 0, GCornerNone);

  // Draw braid without image on Aplite to save memory
  const int size = 2;
  const int start_x = size;
  const int end_x = rect.size.w - size;
  const int start_y = rect.origin.y + size;
  const int end_y = rect.origin.y + rect.size.h - size;
  for (int y = start_y; y < end_y; y += size) {
    const bool offset = ((y - start_y) / size) % 2 == 1;
    for (int x = start_x + (offset ? size : 0); x < end_x; x += (2 * size)) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(
        ctx,
        GRect(x, y, size, size),
        0,
        GCornerNone
      );
    }
  }
#endif
}

// Like menu_cell_basic_draw but with larger subtitle
void util_menu_cell_draw(GContext *ctx, Layer *layer, char *title, char *desc) {
  // TODO: Can we use ContentSize here without layout issues?
  //       It may conflict with pebble-scalable font system
  //
  // Somehow removing this consumes 200B more memory!?
  PreferredContentSize content_size = preferred_content_size();
  // APP_LOG(APP_LOG_LEVEL_INFO, "content_size: %d", (int)content_size);

  // Medium or smaller (rare?), use regular rendering
  if (content_size <= PreferredContentSizeMedium) {
    menu_cell_basic_draw(ctx, layer, title, desc, NULL);
    return;
  }

  // Else, use larger one
  GRect title_rect = GRect(
    scl_x(30),
    scl_y_pp({.o = -30, .e = -10}),
    PS_DISP_W,
    100
  );
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
      GRect(
        scl_x(30),
        scl_y_pp({.o = 110, .e = 130}),
        PS_DISP_W,
        100
      ),
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

  const int hint_x = PS_DISP_W - (HINT_W / 2);

  if (hints[0]) {
    // Top hint
    const int top_y = PS_DISP_H / 6 - (HINT_H / 2);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(
      ctx, GRect(hint_x, top_y, HINT_W, HINT_H),
      3,
      GCornersAll
    );

    // TODO: Ugly, can't configure long press emphasis but only used in one place for now.
    graphics_context_set_fill_color(ctx, GColorWhite);
    const GPoint select_center = {
      .x = hint_x + (HINT_W / 2),
      .y = top_y + (HINT_H / 2)
    };
    graphics_fill_circle(ctx, select_center, scl_x(20));
  }

  if (hints[1]) {
    // Middle hint
    const int middle_y = (PS_DISP_H / 2) - (HINT_H / 2);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(
      ctx, GRect(hint_x, middle_y, HINT_W, HINT_H),
      3,
      GCornersAll
    );
  }

  if (hints[2]) {
    // Bottom hint
    const int bottom_y = ((5 * PS_DISP_H) / 6) - (HINT_H / 2);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(
      ctx, GRect(hint_x, bottom_y, HINT_W, HINT_H),
      3,
      GCornersAll
    );
  }
}

////////////////////////////////////////// Animation Utils /////////////////////////////////////////

#if !defined(PBL_PLATFORM_APLITE)
static Animation *s_animation;
static bool s_animating; // For now, we can assume only one is every active

void util_stop_animation() {
  if (s_animation) {
    animation_unschedule(s_animation);
    animation_destroy(s_animation);
    s_animation = NULL;
  }
}

int util_anim_percentage(AnimationProgress dist_normalized, int max) {
  return (max * dist_normalized) / ANIMATION_NORMALIZED_MAX;
}

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;

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
