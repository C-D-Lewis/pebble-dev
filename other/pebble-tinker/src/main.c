#include <pebble.h>
#include "cl_util.h"

#define APP_TITLE_HEIGHT 16
#define HEADER_HEIGHT 16
#define CELL_HEIGHT 20
#define TOTAL_PINS 8	//D0 - D7

//Keys
enum
{
	PIN_OFF = 0,
	PIN_ON = 1,
	PIN_EVENT = 2
};

//Pin states
enum
{
	STATE_OFF = 0,
	STATE_ON = 1,
	STATE_AWAITING_ON = -1,
	STATE_AWAITING_OFF = -2
};

//Pin object
struct Pin {
	int state;
	InverterLayer *layer;
};

//UI elements
static Window *window;
static BitmapLayer *background_layer;
static GBitmap *background_bitmap;
static InverterLayer *cursor_layer;

//App State
static struct Pin *pins[TOTAL_PINS];
static int current_pin = 0;

/*********************************** Pin Structure ******************************************/

static struct Pin* pin_create()
{
	struct Pin *this = malloc(sizeof(struct Pin));
	this->state = STATE_OFF;

	this->layer = inverter_layer_create(GRect(0, 0, 0, 0));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(this->layer));

	return this;
}

static void pin_destroy(struct Pin *this)
{
	inverter_layer_destroy(this->layer);

	free(this);
}

/****************************** App Message callbacks ***************************************/

static void process_tuple(Tuple *t)
{
	int key = t->key;
	int value = t->value->int32;

	switch(key)
	{
	case PIN_EVENT:
		//Toggle achieved
		switch(pins[value]->state)
		{
		case STATE_AWAITING_ON:
			pins[value]->state = STATE_ON;

			//Animate - layer, start, finish, duration, delay
			cl_animate_layer(inverter_layer_get_layer(pins[value]->layer), GRect(23, 22 + (value * 14), 6, 7), GRect(17, 22 + (value * 14), 12, 7), 500, 0);
			break;
		case STATE_AWAITING_OFF:
			pins[value]->state = STATE_OFF;

			//Animate
			cl_animate_layer(inverter_layer_get_layer(pins[value]->layer), GRect(23, 22 + (value * 14), 6, 7), GRect(29, 22 + (value * 14), 0, 7), 500, 0);
			break;
		}
		break;
	default:
		cl_applog("Unknown key!");
		break;
	}
}

static void in_received_handler(DictionaryIterator *iter, void *context) 
{
	Tuple *t = dict_read_first(iter);
	if(t)
	{
		process_tuple(t);
	}

	while(t)
	{
		t = dict_read_next(iter);
		if(t)
		{
			process_tuple(t);
		}
	}
}

/****************************** Clicks ***********************************************/

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
	//Scroll up
	if(current_pin > 0)
	{
		current_pin--;

		layer_set_frame(inverter_layer_get_layer(cursor_layer), GRect(31, 20 + (current_pin * 14), 4, 11));
	}
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
	//Scroll down
	if(current_pin < TOTAL_PINS - 1)
	{
		current_pin++;

		layer_set_frame(inverter_layer_get_layer(cursor_layer), GRect(31, 20 + (current_pin * 14), 4, 11));
	}
}

static void single_click_handler(ClickRecognizerRef recognizer, void *context)
{
	//Send to Core
	if(pins[current_pin]->state == STATE_OFF)
	{
		//Awaiting ON
		cl_send_int(PIN_ON, current_pin);
		pins[current_pin]->state = STATE_AWAITING_ON;

		cl_animate_layer(inverter_layer_get_layer(pins[current_pin]->layer), GRect(29, 22 + (current_pin * 14), 0, 7), GRect(23, 22 + (current_pin * 14), 6, 7), 500, 0);
	}
	else if(pins[current_pin]->state == STATE_ON)
	{
		//Awaiting OFF
		cl_send_int(PIN_OFF, current_pin);
		pins[current_pin]->state = STATE_AWAITING_OFF;

		cl_animate_layer(inverter_layer_get_layer(pins[current_pin]->layer), GRect(17, 22 + (current_pin * 14), 12, 7), GRect(23, 22 + (current_pin * 14), 6, 7), 500, 0);
	}
}

static void click_config_provider(void *context)
{
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)single_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_click_handler);
}

/******************************** Window Lifecycle ***********************************/

static void window_load(Window *window) 
{
	//Background
	background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
	background_layer = bitmap_layer_create(GRect(0, 0, 144, 152));
	bitmap_layer_set_bitmap(background_layer, background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));

	//Create InverterLayers
	for(int i = 0; i < TOTAL_PINS; i++)
	{
		pins[i] = pin_create();
	}

	//Cursor
	cursor_layer = inverter_layer_create(GRect(31, 20, 4, 11));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(cursor_layer));
}

static void window_unload(Window *window) 
{
	bitmap_layer_destroy(background_layer);
	gbitmap_destroy(background_bitmap);
	inverter_layer_destroy(cursor_layer);

	//Destroy InverterLayers
	for(int i = 0; i < TOTAL_PINS; i++)
	{
		pin_destroy(pins[i]);
	}
}

/***************************** App Liecycle ******************************************/

static void init(void) 
{
	cl_set_debug(true);

	window = window_create();
	window_set_click_config_provider(window, (ClickConfigProvider)click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	//AppMessage
	int 
		in = app_message_inbox_size_maximum(),
		out = app_message_outbox_size_maximum();
	cl_init_app_message(in, out, in_received_handler);
	app_log(APP_LOG_LEVEL_INFO, "C", 0, "I/O: %d/%d", in, out);
	
	//Responiveness+
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	
	window_stack_push(window, true);
}

static void deinit(void) 
{
	//Save power
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
	
	window_destroy(window);
}

int main(void) 
{
	init();
	app_event_loop();
	deinit();
}