#include <pebble.h>
 
Window* window;
TextLayer *text_layer;
InverterLayer *inv_layer;

char buffer[] = "00:00";

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	//Format the buffer string using tick_time as the time source
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	
	//Change the TextLayer text to show the new time!
	text_layer_set_text(text_layer, buffer);
}
 
void window_load(Window *window)
{
	//Time layer
	text_layer = text_layer_create(GRect(0, 53, 132, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	//Inverter layer
	inv_layer = inverter_layer_create(GRect(0, 50, 144, 62));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	//Get a time structure so that the face doesn't start blank
	struct tm *t;
	time_t temp;	
	temp = time(NULL);	
	t = localtime(&temp);	
	
	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
}
 
void window_unload(Window *window)
{
	//We will safely destroy the Window's elements here!
	text_layer_destroy(text_layer);
	
	inverter_layer_destroy(inv_layer);
}
 
void init()
{
	//Initialize the app elements here!
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
	
	window_stack_push(window, true);
}
 
void deinit()
{
	//De-initialize elements here to save memory!
	tick_timer_service_unsubscribe();
	
	window_destroy(window);
}
 
int main(void)
{
	init();
	app_event_loop();
	deinit();
}