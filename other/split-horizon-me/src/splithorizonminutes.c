#include <pebble.h>

//Elements
static Window *window;
static TextLayer *lower_layer, *time_layer, *am_pm_layer, *day_layer, *date_layer, *month_layer;
static InverterLayer *top_shade, *bottom_shade;

//Prototypes
static void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay);
static TextLayer* cl_init_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment);

/******************************** Time functions **********************************/

static void set_time(struct tm *t) {
	int day = t->tm_mday;
	int seconds = t->tm_sec;
	int hours = t->tm_hour;

	//Time string - static keeps it around
	static char hour_text[] = "00:00";
	if(clock_is_24h_style() == true)
	{
		strftime(hour_text, sizeof(hour_text), "%H:%M", t);
	}
	else
	{
		strftime(hour_text, sizeof(hour_text), "%I:%M", t);
	}
	text_layer_set_text(time_layer, hour_text);
	
	
	//AM/PM
	if(clock_is_24h_style() == false) 
	{
		//Show AM/PM marker
		layer_set_frame(text_layer_get_layer(am_pm_layer), GRect(113, 35, 40, 40));
		if(hours < 12)
		{
			text_layer_set_text(am_pm_layer, "AM");
		}
		else
		{
			text_layer_set_text(am_pm_layer, "PM");
		}
	} 
	else
	{
		//Hide it
		layer_set_frame(text_layer_get_layer(am_pm_layer), GRect(0, 0, 0, 0));
	}
		
	//Day string
	static char day_text[] = "Wed";
	strftime(day_text, sizeof(day_text), "%a", t);
	text_layer_set_text(day_layer, day_text);
	
	
	//Date string
	static char date_text[] = "xxxx";
	strftime(date_text, sizeof(date_text), "%eth", t);
	if(day == 1 || day == 21 || day == 31) 
	{	
		//1st, 21st, 31st
		date_text[2] = 's';
		date_text[3] = 't';
	} 
	else if (day == 2 || day == 22) 
	{ 
		//2nd, 22nd
		date_text[2] = 'n';
		date_text[3] = 'd';
	} 
	else if(day == 3 || day == 23) 
	{	
		//3rd, 23rd
		date_text[2] = 'r';
		date_text[3] = 'd';
	}
	text_layer_set_text(date_layer, date_text);

	//Month string
	static char month_text[] = "September";
	strftime(month_text, sizeof(month_text), "%B", t);
	text_layer_set_text(month_layer, month_text);
}

static void tick_handler(struct tm *t, TimeUnits units_changed)
{
	//Get seconds
	int seconds = t->tm_sec;

	//Animate markers and shades
	switch(seconds) {
		case 0:
			//Change time
			set_time(t);
			break;
		case 1:
			//Animate shades out
			animate_layer(inverter_layer_get_layer(top_shade), GRect(0,0,144,84), GRect(0,0,144,0), 400, 0);
			animate_layer(inverter_layer_get_layer(bottom_shade), GRect(0,84,144,84), GRect(0,168,144,0), 400, 0);
			break;
		case 59: 		
			//Animate shades in
			animate_layer(inverter_layer_get_layer(top_shade), GRect(0,0,144,0), GRect(0,0,144,84), 400, 0);
			animate_layer(inverter_layer_get_layer(bottom_shade), GRect(0,168,144,0), GRect(0,84,144,84), 400, 0);
			break;
	}
}

/******************************** Window functions *******************************/

static void window_load(Window *window) {
	//Get resources
	ResHandle font_20 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_20);
	ResHandle font_25 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_25);
	ResHandle font_43 = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_43);

	//Bottom frame
	lower_layer = text_layer_create(GRect(0, 84, 144, 84));
	text_layer_set_background_color(lower_layer, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(lower_layer));

	//TextLayers
	time_layer = cl_init_text_layer(GRect(1, 41, 150, 50), GColorBlack, GColorClear, font_43, GTextAlignmentLeft);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

	am_pm_layer = cl_init_text_layer(GRect(0, 0, 0, 0), GColorBlack, GColorClear, font_20, GTextAlignmentLeft);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(am_pm_layer));

	day_layer = cl_init_text_layer(GRect(1, 75, 150, 50), GColorWhite, GColorClear, font_25, GTextAlignmentLeft);
	text_layer_set_text(day_layer, "MON");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(day_layer));

	date_layer = cl_init_text_layer(GRect(70, 75, 150, 50), GColorWhite, GColorClear, font_25, GTextAlignmentLeft);
	text_layer_set_text(date_layer, "25");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

	month_layer = cl_init_text_layer(GRect(1, 95, 150, 50), GColorWhite, GColorClear, font_20, GTextAlignmentLeft);
	text_layer_set_text(month_layer, "JANUARY");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(month_layer));

	//InverterLayers
	top_shade = inverter_layer_create(GRect(0, 0, 144, 0));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(top_shade));

	bottom_shade = inverter_layer_create(GRect(0, 0, 144, 0));
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(bottom_shade));

	//Init display not blank
	time_t temp = time(NULL);	
	struct tm *t = localtime(&temp);
	set_time(t);
}

static void window_unload(Window *window) {
	text_layer_destroy(lower_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(am_pm_layer);
	text_layer_destroy(day_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(month_layer);

	inverter_layer_destroy(top_shade);
	inverter_layer_destroy(bottom_shade);
}

static void init(void) {
	window = window_create();
	WindowHandlers handlers = {
		.load = window_load,
		.unload = window_unload
	};
	window_set_window_handlers(window, handlers);

	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

	window_stack_push(window, true);
}

static void deinit(void) {
tick_timer_service_unsubscribe();

	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

/****************************** Function Definitions ********************************/

static void on_animation_stopped(Animation *anim, bool finished, void *context)
{
	//Free the memoery used by the Animation
	property_animation_destroy((PropertyAnimation*) anim);
}

static void animate_layer(Layer *layer, GRect start, GRect finish, int duration, int delay)
{
	//Declare animation
	PropertyAnimation *anim = property_animation_create_layer_frame(layer, &start, &finish);
	
	//Set characteristics
	animation_set_duration((Animation*) anim, duration);
	animation_set_delay((Animation*) anim, delay);
	animation_set_curve((Animation*) anim, AnimationCurveEaseInOut);
	
	//Set stopped handler to free memory
	AnimationHandlers handlers = {
		//The reference to the stopped handler is the only one in the array
		.stopped = (AnimationStoppedHandler) on_animation_stopped
	};
	animation_set_handlers((Animation*) anim, handlers, NULL);
	
	//Start animation!
	animation_schedule((Animation*) anim);
}

static TextLayer* cl_init_text_layer(GRect location, GColor colour, GColor background, ResHandle handle, GTextAlignment alignment)
{
	TextLayer *layer = text_layer_create(location);
	text_layer_set_text_color(layer, colour);
	text_layer_set_background_color(layer, background);
	text_layer_set_font(layer, fonts_load_custom_font(handle));
	text_layer_set_text_alignment(layer, alignment);

	return layer;
}
