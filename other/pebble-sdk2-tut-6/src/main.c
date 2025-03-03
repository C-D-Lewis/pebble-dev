#include <pebble.h>
 
Window* window;
TextLayer *title_layer, *location_layer, *temperature_layer, *time_layer;

char location_buffer[64], temperature_buffer[32], time_buffer[32];

enum {
	KEY_LOCATION = 0,
	KEY_TEMPERATURE = 1,
};

void process_tuple(Tuple *t)
{
	//Get key
	int key = t->key;

	//Get integer value, if present
	int value = t->value->int32;

	//Get string value, if present
	char string_value[32];
	strcpy(string_value, t->value->cstring);

	//Decide what to do
	switch(key) {
		case KEY_LOCATION:
			//Location received
			snprintf(location_buffer, sizeof("Location: couldbereallylongname"), "Location: %s", string_value);
			text_layer_set_text(location_layer, (char*) &location_buffer);
			break;
		case KEY_TEMPERATURE:
			//Temperature received
			snprintf(temperature_buffer, sizeof("Temperature: XX \u00B0C"), "Temperature: %d \u00B0C", value);
			text_layer_set_text(temperature_layer, (char*) &temperature_buffer);
			break;
	}

	//Set time this update came in
	time_t temp = time(NULL);	
	struct tm *tm = localtime(&temp);
	strftime(time_buffer, sizeof("Last updated: XX:XX"), "Last updated: %H:%M", tm);
	text_layer_set_text(time_layer, (char*) &time_buffer);
}

static void in_received_handler(DictionaryIterator *iter, void *context) 
{
	(void) context;
	
	//Get data
	Tuple *t = dict_read_first(iter);
	while(t != NULL)
	{
		process_tuple(t);
		
		//Get next
		t = dict_read_next(iter);
	}
}

static TextLayer* init_text_layer(GRect location, GColor colour, GColor background, const char *res_id, GTextAlignment alignment)
{
	TextLayer *layer = text_layer_create(location);
	text_layer_set_text_color(layer, colour);
	text_layer_set_background_color(layer, background);
	text_layer_set_font(layer, fonts_get_system_font(res_id));
	text_layer_set_text_alignment(layer, alignment);

	return layer;
}
 
void window_load(Window *window)
{
	title_layer = init_text_layer(GRect(5, 0, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentLeft);
	text_layer_set_text(title_layer, "Openweathermap.org");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(title_layer));

	location_layer = init_text_layer(GRect(5, 30, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentLeft);
	text_layer_set_text(location_layer, "Location: N/A");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(location_layer));

	temperature_layer = init_text_layer(GRect(5, 60, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentLeft);
	text_layer_set_text(temperature_layer, "Temperature: N/A");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer));

	time_layer = init_text_layer(GRect(5, 90, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentLeft);
	text_layer_set_text(time_layer, "Last updated: N/A");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
}
 
void window_unload(Window *window)
{	
	text_layer_destroy(title_layer);
	text_layer_destroy(location_layer);
	text_layer_destroy(temperature_layer);
	text_layer_destroy(time_layer);
}

void send_int(uint8_t key, uint8_t cmd)
{
	DictionaryIterator *iter;
 	app_message_outbox_begin(&iter);
 	
 	Tuplet value = TupletInteger(key, cmd);
 	dict_write_tuplet(iter, &value);
 	
 	app_message_outbox_send();
}

void tick_callback(struct tm *tick_time, TimeUnits units_changed)
{
	//Every five minutes
	if(tick_time->tm_min % 5 == 0)
	{
		//Send an arbitrary message, the response will be handled by in_received_handler()
		send_int(5, 5);
	}
}
 
void init()
{
	window = window_create();
	WindowHandlers handlers = {
		.load = window_load,
		.unload = window_unload
	};
	window_set_window_handlers(window, handlers);

	//Register AppMessage events
	app_message_register_inbox_received(in_received_handler);					 
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());		//Largest possible input and output buffer sizes
	
	//Register to receive minutely updates
	tick_timer_service_subscribe(MINUTE_UNIT, tick_callback);

	window_stack_push(window, true);
}
 
void deinit()
{
	tick_timer_service_unsubscribe();
	
	window_destroy(window);
}
 
int main(void)
{
	init();
	app_event_loop();
	deinit();
}
