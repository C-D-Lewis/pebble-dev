#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "mod.h"

#define MY_UUID { 0x6f, 0x8f, 0xd9, 0xff, 0x1a, 0xf9, 0x4e, 0x32, 0x8b, 0x21, 0x8c, 0xd6, 0x42, 0x94, 0x9f, 0x75 }

PBL_APP_INFO(MY_UUID, "Divider Large", "Chris Lewis", 1, 0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;
BmpContainer backgroundContainer;
TextLayer batteryLayer, hoursLayer, minutesLayer, outputLayer, vLayer;
InverterLayer secondsLayer;

static char r1Text[2] = "00";
static char r2Text[2] = "00";
static char outText[3] = "000";
static char battText[2] = "00";
static int x = 21, y = 78, w = 3, h = 3;
static bool setup = false;

//Set container image 
void set_container_image(BmpContainer *bmp_container, const int resource_id, GPoint origin) {

	layer_remove_from_parent(&bmp_container->layer.layer);
	bmp_deinit_container(bmp_container);

	bmp_init_container(resource_id, bmp_container);

	GRect frame = layer_get_frame(&bmp_container->layer.layer);
	frame.origin.x = origin.x;
	frame.origin.y = origin.y;
	layer_set_frame(&bmp_container->layer.layer, frame);

	layer_add_child(&window.layer, &bmp_container->layer.layer);
}

//Handle function called every second
void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
	(void)ctx;

	PblTm time;
	get_time(&time);

	int days = time.tm_mday;
	int hours = time.tm_hour;
	int minutes = time.tm_min;
	int seconds = time.tm_sec;

	//Update minute resistors and output voltage
	if(((seconds % 60) == 0) || (setup == false)) {
		float fDays = 0.0F + days;
		float fHours = 0.0F + hours;
		float fMinutes = 0.0F + minutes;

		//Day of the month battery
		static char dayText[] = "00";
		string_format_time(dayText, sizeof(dayText), "%d", &time);
		strcpy(battText, dayText);

		//Hour resistor
		static char hourText[] = "00";
		string_format_time(hourText, sizeof(hourText), "%H", &time);
		strcpy(r1Text, hourText);

		//Minute resistor
		static char minuteText[] = "00";
		string_format_time(minuteText, sizeof(minuteText), "%M", &time);
		strcpy(r2Text, minuteText);
	
		//Output voltage
		float voltage = (fMinutes/(fHours + fMinutes)) * fDays;
	
		//Show output voltage
		int digit2 = mod(voltage, 10);
		voltage /= 10.0F;
		int digit1 = mod(voltage, 10);
		
		if(digit1 > 0)	//Two digits
			outText[0] = '0' + digit1;
		else	//One digit
			outText[0] = ' ';
	
		outText[1] = '0' + digit2;
		outText[2] = 'V';
		
		//Set layers text
		text_layer_set_text(&batteryLayer, battText);
		text_layer_set_text(&hoursLayer, r1Text);
		text_layer_set_text(&minutesLayer, r2Text);
		text_layer_set_text(&outputLayer, outText);
	}

	//Update moving Inverterlayer secondsLayer
	if((seconds == 0) || (setup == false)) {	//Initial setup
		x = 21;
		y = 78;
		setup = true;
	}
	if(seconds < 10) {	//Move up
		y -= 6;
		if(x != 21)	//Keep in sync after watchface changed - need a handler function for this?
			x = 21;
	}
	if(seconds == 10)	//Setup going right
		y = 16;
	if((seconds > 10) && (seconds < 20)) {	//Move right
		x += 7;
		if(y != 16)
			y = 16;
	}
	if(seconds == 20)	//Setup going down
		x = 94;
	if((seconds > 20) && (seconds < 40)) {	//Move down
		y += 7;
		if(x != 94)
			x = 94;
	}
	if(seconds == 40)	//Setup going left
		y = 151;
	if((seconds > 40) && (seconds < 51)) {	//Move left
		x -= 7;
		if(y != 151)
		 y = 151;
	}
	if(seconds == 51)	//Setup going back up
		x = 21;
	if((seconds > 51) && (seconds < 60)) {		//Move up
		y -= 7;
		if(x != 21)
			x = 21;
	}

	//Draw moving thing
	layer_set_frame((Layer *)&secondsLayer, GRect(x, y, w, h));
}

//Init a textLayer
void init_text_layer(TextLayer *textLayer, GRect frame, GFont font) {
	text_layer_init(textLayer, frame);
	text_layer_set_background_color(textLayer, GColorBlack);
	text_layer_set_text_color(textLayer, GColorWhite);
	text_layer_set_font(textLayer, font);
	text_layer_set_text_alignment(textLayer, GTextAlignmentLeft);
}
	

//Resource initialisation handle function
void handle_init(AppContextRef ctx) {
	(void)ctx;

	//Setup window layer
	window_init(&window, "Window Name");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);

	///Init resources
	resource_init_current_app(&APP_RESOURCES);

	//Init background
	set_container_image(&backgroundContainer, RESOURCE_ID_BACKGROUND_IMAGE, GPoint(0, 0));
	layer_add_child(&window.layer, &backgroundContainer.layer.layer);
	
	//Init textLayers
	init_text_layer(&batteryLayer, GRect(40, 73, 30, 30), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_20)));//
	init_text_layer(&hoursLayer, GRect(30, 30, 38, 30), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_30)));
	init_text_layer(&minutesLayer, GRect(30, 98, 38, 30), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_30)));
	init_text_layer(&outputLayer, GRect(106, 57, 100, 20), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_20)));
	init_text_layer(&vLayer, GRect(63, 73, 20, 20), fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_20)));//
	text_layer_set_text(&vLayer, "V");//
		
	//Add textLayers to window
	layer_add_child(&window.layer, &batteryLayer.layer);
	layer_add_child(&window.layer, &hoursLayer.layer);
	layer_add_child(&window.layer, &minutesLayer.layer);
	layer_add_child(&window.layer, &outputLayer.layer);
	layer_add_child(&window.layer, &vLayer.layer);

	//Init moving inverter layer
	inverter_layer_init(&secondsLayer, window.layer.frame);
	layer_add_child(&window.layer, &secondsLayer.layer);

	//Initial calculation
	handle_second_tick(ctx, NULL);
}

//Resource deinitialisation handle function
void handle_deinit(AppContextRef ctx) {
	(void)ctx;

	//deinit images here - good practice for memory usage
	bmp_deinit_container(&backgroundContainer);
	
/*	//deinit textLayers
	text_layer_deinit(&batteryLayer);	//Not in Pebble SDK 1. Need to migrate soon
	text_layer_deinit(&hoursLayer);
	text_layer_deinit(&minutesLayer);
	text_layer_deinit(&outputLayer);
	text_layer_deinit(&vLayer); */
}

void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		//Resource initialisation handle functions
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,

		// Handle time update handle functions
		.tick_info = {
			.tick_handler = &handle_second_tick,
			.tick_units = SECOND_UNIT
		}
	};
	app_event_loop(params, &handlers);
}
