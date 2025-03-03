#include <pebble.h>
 
Window* window;
TextLayer *text_layer, *square_layer;
InverterLayer *inv_layer;
GBitmap *future_bitmap, *past_bitmap;
BitmapLayer *future_layer, *past_layer;
AppTimer *timer;
const int square_size = 10;
const int delta = 40;
int dx = 1;

char buffer[] = "00:00";

void on_animation_stopped(Animation *anim, bool finished, void *context)
{
	//Free the memoery used by the Animation
	property_animation_destroy((PropertyAnimation*) anim);
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
	//Declare animation
	PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
	
	//Set characteristics
	animation_set_duration((Animation*) anim, duration);
	animation_set_delay((Animation*) anim, delay);
	
	//Set stopped handler to free memory
	AnimationHandlers handlers = {
		//The reference to the stopped handler is the only one in the array
		.stopped = (AnimationStoppedHandler) on_animation_stopped
	};
	animation_set_handlers((Animation*) anim, handlers, NULL);
	
	//Start animation!
	animation_schedule((Animation*) anim);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	//Format the buffer string using tick_time as the time source
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	
	int seconds = tick_time->tm_sec;
	
	if(seconds == 59)
	{
		//Slide offscreen to the right
		GRect start = GRect(0, 53, 144, 168);
		GRect finish = GRect(144, 53, 144, 168);
		animate_layer(text_layer_get_layer(text_layer), &start, &finish, 300, 500);
	}
	
	else if(seconds == 0)
	{
		//Change the TextLayer text to show the new time!
		text_layer_set_text(text_layer, buffer);

		//Slide onscreen from the left
		GRect start = GRect(-144, 53, 144, 168);
		GRect finish = GRect(0, 53, 144, 168);
		animate_layer(text_layer_get_layer(text_layer), &start, &finish, 300, 500);
	}
	
	else
	{
		//Change the TextLayer text to show the new time!
		text_layer_set_text(text_layer, buffer);
	}
}

void timer_callback(void *data) {	
	//Get current position
	GRect current = layer_get_frame(text_layer_get_layer(square_layer));
	
	//Check to see if we have hit the edges
	if(current.origin.x > 144 - square_size)
	{
		dx = -1;	//Reverse
	}
	else if(current.origin.x < 0)
	{
		dx = 1;	//Forwards
	}

	//Move the square to the next position, modifying the x value
	GRect next = GRect(current.origin.x + dx, current.origin.y, square_size, square_size);
	layer_set_frame(text_layer_get_layer(square_layer), next);
	
	//Register next execution
	timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
}

void window_load(Window *window)
{
	//Load font 
	ResHandle font_handle = resource_get_handle(RESOURCE_ID_IMAGINE_42);
	
	//Time layer
	text_layer = text_layer_create(GRect(0, 53, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	//Inverter layer
	inv_layer = inverter_layer_create(GRect(0, 50, 144, 62));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	//Load bitmaps into GBitmap structures
	//The ID you chose when uploading is prefixed with 'RESOURCE_ID_'
	future_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FUTURE);
	past_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PAST);
		
	//Create BitmapLayers to show GBitmaps and add to Window
	//Sample images are 144 x 50 pixels
	future_layer = bitmap_layer_create(GRect(0, 0, 144, 50));
	bitmap_layer_set_bitmap(future_layer, future_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(future_layer));
		
	past_layer = bitmap_layer_create(GRect(0, 112, 144, 50));
	bitmap_layer_set_bitmap(past_layer, past_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(past_layer));
	
	//Create the square layer
	square_layer = text_layer_create(GRect(0, 55, square_size, square_size));
	text_layer_set_background_color(square_layer, GColorWhite);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(square_layer));
	
	//Get a time structure so that the face doesn't start blank
	struct tm *t;
	time_t temp;	
	temp = time(NULL);	
	t = localtime(&temp);	
	
	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
	
	//Start the square moving
	timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
}
 
void window_unload(Window *window)
{
	//Destroy TextLayer
	text_layer_destroy(text_layer);
	
	//Destroy InverterLayer
	inverter_layer_destroy(inv_layer);
	
	//Destroy GBitmaps
	gbitmap_destroy(future_bitmap);
	gbitmap_destroy(past_bitmap);
	
	//Destroy BitmapLayers
	bitmap_layer_destroy(future_layer);
	bitmap_layer_destroy(past_layer);
	
	//Cancel timer
	app_timer_cancel(timer);
	
	//Destroy square layer
	text_layer_destroy(square_layer);
}
 
void init()
{
	//Initialize the app elements here!
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
	
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