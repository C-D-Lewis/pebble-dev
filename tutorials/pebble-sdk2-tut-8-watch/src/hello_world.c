#include <pebble.h>
	
enum {
	KEY_BUTTON_EVENT = 0,
	BUTTON_EVENT_UP = 1,
	BUTTON_EVENT_DOWN = 2,
	BUTTON_EVENT_SELECT = 3,
	KEY_VIBRATION = 4
};

static Window *window;
static TextLayer *text_layer;

void send_int(uint8_t key, uint8_t cmd)
{
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
     
    Tuplet value = TupletInteger(key, cmd);
    dict_write_tuplet(iter, &value);
     
    app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "Select");
	send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_SELECT);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "Up");
	send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_UP);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	text_layer_set_text(text_layer, "Down");
	send_int(KEY_BUTTON_EVENT, BUTTON_EVENT_DOWN);
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
	text_layer_set_text(text_layer, "Press a button");
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
}

static void in_received_handler(DictionaryIterator *iter, void *context) 
{
	Tuple *t = dict_read_first(iter);
	if(t)
	{
		vibes_short_pulse();
	}
}

static void init(void) {
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	//Register AppMessage events
	app_message_register_inbox_received(in_received_handler);					 
	app_message_open(512, 512);		//Large input and output buffer sizes
	
	const bool animated = true;
	window_stack_push(window, animated);
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
