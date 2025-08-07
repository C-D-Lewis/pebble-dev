#include "card.h"

static bool match(char *subject, char *p1, char *p2) {
  return strstr(subject, p1) != NULL || strstr(subject, p2) != NULL;
}

static void battery_bar_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, grect_inset(bounds, GEdgeInsets(1)), 0, GCornerNone);
}

/**
 * Create a Card object and all subelements
 */
Card* card_add(Window *window, char *location_buff_ptr, char* conditions_buff_ptr) {
  GRect bounds = CARD_SPAWN_LOCATION;
  Layer *root_layer = window_get_root_layer(window);

  // Allocate
  Card *this = malloc(sizeof(Card));
  this->background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CARD);
  this->background_layer = bitmap_layer_create(GRect(bounds.origin.x, bounds.origin.y + bounds.size.h - 10, 144, 109));
  this->time_layer = cl_text_layer_create(GRect(bounds.origin.x + 10, bounds.origin.y, bounds.size.w - 10, 50), GColorBlack, GColorClear, false, NULL, FONT_KEY_BITHAM_42_LIGHT, GTextAlignmentLeft);
  this->date_layer = cl_text_layer_create(GRect(bounds.origin.x + 10, bounds.origin.y + 40, 144, 30), GColorBlack, GColorClear, false, NULL, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);
  this->battery_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY);
  this->battery_bg_layer = bitmap_layer_create(GRect(bounds.origin.x + 125, bounds.origin.y + 15, BAT_ICN_W, 22));
  this->battery_bar_layer = layer_create(GRect(bounds.origin.x + 125, bounds.origin.y + 15, BAT_ICN_W, BAT_ICN_H));
  layer_set_update_proc(this->battery_bar_layer, battery_bar_update_proc);
  this->bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT);
  this->bt_layer = bitmap_layer_create(GRect(bounds.origin.x + 125, bounds.origin.y + 40, 9, 16));
  this->weather_layer = bitmap_layer_create(GRect(bounds.origin.x + 10, bounds.origin.y + 63, 22, 22));
  this->conditions_layer = cl_text_layer_create(GRect(bounds.origin.x + 36, bounds.origin.y + 53, 109, 30), GColorBlack, GColorClear, false, NULL, FONT_KEY_GOTHIC_24, GTextAlignmentLeft);
  this->location_layer = cl_text_layer_create(GRect(bounds.origin.x + 36, bounds.origin.y + 75, 109, 30), GColorBlack, GColorClear, false, NULL, FONT_KEY_GOTHIC_18, GTextAlignmentLeft);

  // Setup shadow
  bitmap_layer_set_bitmap(this->background_layer, this->background_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(this->background_layer));

  // Setup text
  this->time_buff = malloc(sizeof("00:00"));
  strftime(this->time_buff, sizeof("00:00"), clock_is_24h_style() ? "%H:%M" : "%I:%M", cl_get_time());
  text_layer_set_text(this->time_layer, this->time_buff);
  layer_add_child(root_layer, text_layer_get_layer(this->time_layer));

  // Setup date
  this->date_buff = malloc(sizeof("September 30 2014"));
  strftime(this->date_buff, sizeof("September 30 2014"), "%B %d %Y", cl_get_time());
  text_layer_set_text(this->date_layer, this->date_buff);
  layer_add_child(root_layer, text_layer_get_layer(this->date_layer));

  // Setup battery
  bitmap_layer_set_bitmap(this->battery_bg_layer, this->battery_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(this->battery_bg_layer));
  layer_add_child(root_layer, this->battery_bar_layer);

  // BT
  bitmap_layer_set_bitmap(this->bt_layer, this->bt_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(this->bt_layer));

  // Setup weather icon - if buffer is not a conditions i.e: not fetched
  if (strlen(conditions_buff_ptr) < 1) {
    this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UNKNOWN);
  } else {
    struct tm *time = cl_get_time();

    if (time->tm_hour > 6 && time->tm_hour < 18) {
      // Day
      if (match(conditions_buff_ptr, "Clear", "clear") || match(conditions_buff_ptr, "Sun", "sun")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUN);
      } else if (match(conditions_buff_ptr, "Rain", "rain")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN);
      } else if (match(conditions_buff_ptr, "Cloud", "cloud") || match(conditions_buff_ptr, "Overcast", "overcast")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUD);
      } else if (match(conditions_buff_ptr, "Wind", "wind")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WIND);
      } else if (match(conditions_buff_ptr, "Thunder", "thunder")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDER);
      } else if (match(conditions_buff_ptr, "Mist", "mist")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIST);
      } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown weather type: %s", conditions_buff_ptr);
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UNKNOWN);
      }
    } else {
      // Night
      if (match(conditions_buff_ptr, "Clear", "clear") || match(conditions_buff_ptr, "Sun", "sun")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MOON);
      } else if (match(conditions_buff_ptr, "Rain", "rain")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN);
      } else if (match(conditions_buff_ptr, "Cloud", "cloud") || match(conditions_buff_ptr, "Overcast", "overcast")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUD_NIGHT);
      } else if (match(conditions_buff_ptr, "Wind", "wind")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WIND_NIGHT);
      } else if (match(conditions_buff_ptr, "Thunder", "thunder")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDER);
      } else if (match(conditions_buff_ptr, "Mist", "mist")) {
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MIST_NIGHT);
      } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown weather type: %s", conditions_buff_ptr);
        this->weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_UNKNOWN);
      }
    }
  }
  bitmap_layer_set_bitmap(this->weather_layer, this->weather_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(this->weather_layer));

  // Setup weather conditions 
  text_layer_set_overflow_mode(this->conditions_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(this->conditions_layer, conditions_buff_ptr);
  layer_add_child(root_layer, text_layer_get_layer(this->conditions_layer));

  // Setup weather conditions
  text_layer_set_overflow_mode(this->location_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(this->location_layer, location_buff_ptr);

  layer_add_child(root_layer, text_layer_get_layer(this->location_layer));

  return this;
}

/**
 * Destroy a Card and all subelements
 */
void card_destroy(Card *this) {
  layer_destroy(this->battery_bar_layer);
  
  bitmap_layer_destroy(this->battery_bg_layer);
  bitmap_layer_destroy(this->background_layer);
  bitmap_layer_destroy(this->bt_layer);
  bitmap_layer_destroy(this->weather_layer);
  
  gbitmap_destroy(this->background_bitmap);
  gbitmap_destroy(this->battery_bitmap);
  gbitmap_destroy(this->bt_bitmap);
  gbitmap_destroy(this->weather_bitmap);

  text_layer_destroy(this->time_layer);
  text_layer_destroy(this->date_layer);
  text_layer_destroy(this->conditions_layer);
  text_layer_destroy(this->location_layer);

  free(this->time_buff);
  free(this->date_buff);
  free(this);
}

/*
 * Internal Animation disposal
 */
static void on_animation_stopped(Animation *anim, bool finished, void *context) {
#ifdef PBL_PLATFORM_APLITE
  property_animation_destroy((PropertyAnimation*)anim);
 #endif
}

/*
 * Animate a layer with duration
 */
static void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay, AnimationCurve curve) {
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
  
  animation_set_duration((Animation*)anim, duration);
  animation_set_curve((Animation*)anim, curve);
  animation_set_delay((Animation*)anim, delay);
  
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler)on_animation_stopped
  };
  animation_set_handlers((Animation*)anim, handlers, NULL);
  
  animation_schedule((Animation*)anim);
}

/**
 * Animate entire card
 */
static void card_animate(Card *this, GRect start, GRect finish, int duration, int delay, AnimationCurve curve) {
  // Animate background with all children
  animate_layer(bitmap_layer_get_layer(this->background_layer), start, finish, duration, delay, curve);

  // Animate using offset - time
  animate_layer(
    text_layer_get_layer(this->time_layer), 
    GRect(start.origin.x + 10, start.origin.y, start.size.w - 10, 50), 
    GRect(finish.origin.x + 10, finish.origin.y, finish.size.w - 10, 50), 
    duration, delay, curve
  );

  // Date
  animate_layer(
    text_layer_get_layer(this->date_layer), 
    GRect(start.origin.x + 10, start.origin.y + 40, start.size.w - 10, 50),  
    GRect(finish.origin.x + 10, finish.origin.y + 40, finish.size.w - 10, 50), 
    duration, delay, curve
  );

  // Battery - calculate height of bar
  BatteryChargeState state = battery_state_service_peek();
  int height = state.charge_percent * BAT_ICN_H / 100;
  animate_layer(
    bitmap_layer_get_layer(this->battery_bg_layer),
    GRect(start.origin.x + 125, start.origin.y + 13, BAT_ICN_W, BAT_ICN_H),
    GRect(finish.origin.x + 125, finish.origin.y + 15, BAT_ICN_W, BAT_ICN_H),
    duration, delay, curve
  );
  animate_layer(
    this->battery_bar_layer,
    GRect(start.origin.x + 125, start.origin.y + 15 + (BAT_ICN_H - height), BAT_ICN_W, height),
    GRect(finish.origin.x + 125, finish.origin.y + 15 + (BAT_ICN_H - height), BAT_ICN_W, height),
    duration, delay, curve
  );

  // BT
  if (bluetooth_connection_service_peek()) {
    animate_layer(
      bitmap_layer_get_layer(this->bt_layer),
      GRect(start.origin.x + 125, start.origin.y + 40, 9, 16),
      GRect(finish.origin.x + 125, finish.origin.y + 40, 9, 16),
      duration, delay, curve
    );
  } else {
    // Hide it
    layer_set_frame(bitmap_layer_get_layer(this->bt_layer), GRect(0, 0, 0, 0));
  }

  // Weather
  animate_layer(
    bitmap_layer_get_layer(this->weather_layer),
    GRect(start.origin.x + 10, start.origin.y + 63, 22, 22),
    GRect(finish.origin.x + 10, finish.origin.y + 63, 22, 22),
    duration, delay, curve
  );

  // Conditions
  animate_layer(
    text_layer_get_layer(this->conditions_layer),
    GRect(start.origin.x + 36, start.origin.y + 53, 109, 30),
    GRect(finish.origin.x + 36, finish.origin.y + 53, 109, 33),
    duration, delay, curve
  );

  // Location
  animate_layer(
    text_layer_get_layer(this->location_layer),
    GRect(start.origin.x + 36, start.origin.y + 75, 109, 30),
    GRect(finish.origin.x + 36, finish.origin.y + 75, 109, 33),
    duration, delay, curve
  );
}

/**
 * Animate card from bottom
 */
void card_spawn(Card *this) {
  card_animate(this, CARD_SPAWN_LOCATION, CARD_RESTING_LOCATION, 500, 0, AnimationCurveEaseIn);
}

/**
 * Animate card up to top
 */
void card_despawn(Card *this) {
  card_animate(this, CARD_RESTING_LOCATION, CARD_DESPAWN_LOCATION, 500, 0, AnimationCurveEaseOut);
}