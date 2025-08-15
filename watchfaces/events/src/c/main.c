#include <pebble.h>
#include "libs/cl_util.h"

#if defined(PBL_PLATFORM_EMERY)
  #define TOTAL_EVENTS 15
#else
  #define TOTAL_EVENTS 11
#endif
#define MAX_EVENT_LENGTH 32
#define FONT_SIZE 15
#define SHIFT 15
#define ANIMATION_DURATION 300

// Structure to align TextLayer and content array
typedef struct {
  TextLayer *layer;
  char *content;
} Item;

// Layout
static Window* window;
static Item *items[TOTAL_EVENTS];

// State
static GFont font;
static bool 
  last_connected = false,
  last_is_charging = false,
  last_is_plugged = false,
  push_in_progress = false;
static char *last_message;

// Prototypes
static void push_event(const char *message);

/************************* Items lifecycle ***************************/

static Item* item_create(int i) {
  Item *this = malloc(sizeof(Item));

  // Layer
  this->layer = cl_text_layer_create(
    GRect(5, (i * SHIFT) + SHIFT, 134, 20),
    GColorWhite,
    GColorClear,
    true,
    font,
    NULL,
    GTextAlignmentLeft
  );
  text_layer_set_overflow_mode(this->layer, GTextOverflowModeTrailingEllipsis);

  // Storage
  this->content = malloc(MAX_EVENT_LENGTH * sizeof(char));

  return this;
}

static void item_destroy(Item *this) {
  text_layer_destroy(this->layer);
  free(this->content);
  free(this);

  this = NULL;
}

static void items_create() {
  for (int i = 0; i < TOTAL_EVENTS; i++) {
    items[i] = item_create(i);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(items[i]->layer));
  }
}

static void items_destroy() {
  for (int i = 0; i < TOTAL_EVENTS; i++) {
    if (items[i] != NULL) {
      item_destroy(items[i]);
    }
  }
}

/************************** Push Event Lifecycle ***************************/

static void push_event_handler(Animation *anim, bool finished, void *context) {
  // Destroy 0
  item_destroy(items[0]);

  // Re-assign all up 1 except last
  for (int i = 0; i < TOTAL_EVENTS - 1; i++) {
    items[i] = items[i + 1];
  }

  // Init new at 10
  items[TOTAL_EVENTS - 1] = item_create(TOTAL_EVENTS - 1);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(items[TOTAL_EVENTS - 1]->layer));

  // Set text
  snprintf(items[TOTAL_EVENTS - 1]->content, MAX_EVENT_LENGTH * sizeof(char), "%s", last_message);
  text_layer_set_text(items[TOTAL_EVENTS - 1]->layer, items[TOTAL_EVENTS - 1]->content);

  // Animate in
  GRect start = layer_get_frame(text_layer_get_layer(items[TOTAL_EVENTS - 1]->layer));
  GRect finish = GRect(start.origin.x, start.origin.y - SHIFT, start.size.w, start.size.h);
  cl_animate_layer(text_layer_get_layer(items[TOTAL_EVENTS - 1]->layer), start, finish, ANIMATION_DURATION, 0, NULL);

  // Finally
  property_animation_destroy((PropertyAnimation*) anim);
  push_in_progress = false;
}

static void push_event_retry(void *context) {
  // Convert back to char *arr (cast, then deref pointer)
  char *message = (char*)context;

  // Our turn?
  if (push_in_progress == false) { 
    // Push for real this time
    push_event(message);

    // Free
    free(message);
  } else {
    // Retry later
    // Register with passed message again
    app_timer_register(5000, (AppTimerCallback)push_event_retry, message);
  }
}

static void push_event(const char *message) {
  if (push_in_progress) {
    // Retry - register with temp buffer - will be freed in callback
    char *buffer = malloc(MAX_EVENT_LENGTH * sizeof(char));
    app_timer_register(5000, (AppTimerCallback)push_event_retry, buffer);
    return;
  }
    
  // Stop interacting handlers
  push_in_progress = true;

  // Animate all up
  for (int i = 0; i < TOTAL_EVENTS - 1; i++) {
    GRect start = layer_get_frame(text_layer_get_layer(items[i]->layer));
    GRect finish = GRect(start.origin.x, start.origin.y - SHIFT, start.size.w, start.size.h);
    cl_animate_layer(text_layer_get_layer(items[i]->layer), start, finish, ANIMATION_DURATION, 0, NULL);
  }

  // Animate last with handler
  GRect start = layer_get_frame(text_layer_get_layer(items[TOTAL_EVENTS - 1]->layer));
  GRect finish = GRect(start.origin.x, start.origin.y - SHIFT, start.size.w, start.size.h);
  cl_animate_layer(
    text_layer_get_layer(items[TOTAL_EVENTS - 1]->layer),
    start,
    finish,
    ANIMATION_DURATION,
    0,
    (AnimationStoppedHandler)push_event_handler
  );

  // Store new message for handler
  snprintf(last_message, MAX_EVENT_LENGTH * sizeof(char), "%s", message);
}

/************************ Event Handlers ****************************/

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  static char buff[MAX_EVENT_LENGTH];
  strftime(buff, MAX_EVENT_LENGTH * sizeof(char), "Time %H:%M", tick_time);
  push_event(&buff[0]);
}

static void bt_handler(bool connected) {
  if (last_connected != connected) {
    last_connected = connected;
    vibes_short_pulse();
  }

  push_event(connected ? "Connected": "Disconnected");
}

static void battery_handler(BatteryChargeState charge) {
  static char buff[MAX_EVENT_LENGTH];
  snprintf(buff, MAX_EVENT_LENGTH * sizeof(char), "Battery %d/100", charge.charge_percent);
  push_event(&buff[0]);

  // Changed state?
  if (charge.is_charging != last_is_charging) {
    last_is_charging = charge.is_charging;

    push_event(charge.is_charging ? "Charging" : "Discharging");
  }

  if (charge.is_plugged != last_is_plugged) {
    last_is_plugged = charge.is_plugged;

    push_event(charge.is_plugged ? "Plugged in" : "Un-plugged");
  }
}

static void focus_handler(bool in_focus) {
  push_event(in_focus ? "Gained focus" : "Lost focus");
}

/************************* Main Window *****************************/
 
static void window_load(Window *window) {
  window_set_background_color(window, GColorBlack);

  // Allocations
  last_message = malloc(MAX_EVENT_LENGTH * sizeof(char));
  ResHandle handle = resource_get_handle(RESOURCE_ID_FONT_15);
  font = fonts_load_custom_font(handle);
  
  // Items
  items_create();

  // Events
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler)tick_handler);
  bluetooth_connection_service_subscribe((BluetoothConnectionHandler)bt_handler);
  battery_state_service_subscribe((BatteryStateHandler)battery_handler);
  app_focus_service_subscribe((AppFocusHandler)focus_handler);

  // Show intial state - races currently
  // last_connected = bluetooth_connection_service_peek();
  // bt_handler(last_connected);
  
  // BatteryChargeState init_state = battery_state_service_peek();
  // last_is_charging = init_state.is_charging;
  // last_is_plugged = init_state.is_plugged;
  // battery_handler(init_state);  
}
 
static void window_unload(Window *window) {
  items_destroy();

  // Free
  free(last_message);
  fonts_unload_custom_font(font);

  // Self destroy
  window_destroy(window);
}
 
/************************** App Lifecycle ***************************/

static void init() {
  // Seed
  srand(time(NULL));

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true);
}
 
static void deinit() {
}
 
int main(void) {
  init();
  app_event_loop();
  deinit();
}
