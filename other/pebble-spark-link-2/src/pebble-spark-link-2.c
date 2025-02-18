#include <pebble.h>

#define KEY_BUTTON_STATE 0

static Window *window;
static TextLayer *text_layer;

static void window_load(Window *window) 
{
	//Create TextLayer
	text_layer = text_layer_create(GRect(0, 0, 144, 168));
	text_layer_set_text(text_layer, "Press button on Core pin D0");
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) 
{
	//Destroy TextLayer
	text_layer_destroy(text_layer);
}

static void in_recv_handler(DictionaryIterator *iterator, void *context)
{
	//Get first tuple (should be KEY_BUTTON_STATE)
	Tuple *t = dict_read_first(iterator);

	//If it's there
	if(t)
	{
		if(strcmp("HIGH", t->value->cstring) == 0)
		{
			vibes_short_pulse();
		}
	}
}

static void init(void) 
{
	//Create Window
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	//Prepare AppMessage
	app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

	window_stack_push(window, true);
}

static void deinit(void) 
{
	//Destroy Window
	window_destroy(window);
}

int main(void) 
{
	init();
	app_event_loop();
	deinit();
}
